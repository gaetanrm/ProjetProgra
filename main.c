#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"


sites init(int port){ //Initialisation de tous les sites au démarrage de l'algo
	//Chacun doit envoyer son num aux autres pour savoir qui est le 1, donc qui sera la racine au départ de l'algo même si il n'a pas fait de demande.
	//C'est juste pour que l'algo puisse fonctionner ensuite car il a besoin d'une racine pour cela
	sites moiMeme;
	char* ipPere = "127.0.1.1"; //FAUT CHANGER, FAUT PAS RENTRER LA VALEUR COMME CA
	moiMeme.addr.sin_addr.s_addr = inet_ntoa(**connaitreIP());
	moiMeme.addr.sin_port = htons((short) port);
	moiMeme.addr.sin_family = AF_INET;
	moiMeme.Next.sin_addr.s_addr = NULL;
	moiMeme.Next.sin_port = 0;
	moiMeme.Next.sin_family = AF_INET;
	if (moiMeme.addr.sin_addr.s_addr == ipPere) {
		moiMeme.Pere.sin_addr.s_addr = NULL;
		moiMeme.Pere.sin_port = 0;
		moiMeme.jeton_present = 1;
	} else {
		moiMeme.Pere.sin_addr.s_addr = ipPere; //Valeur aléatoire, faudra s'en occuper plus tard
		moiMeme.Pere.sin_port = htons((short) 8888); //Idem
		moiMeme.Pere.sin_family = AF_INET;
		moiMeme.jeton_present = 0;
	}
	moiMeme.est_demandeur = 0;
	moiMeme.estEn_SC = 0;
	return moiMeme;

}

int envoyerDemande(sites* k){ 	//Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
								//resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre
	(*k).est_demandeur = 1;

	if ((*k).Pere.sin_addr.s_addr == NULL){
		//FAUX (*k).estEn_SC = 1;
		return 1;
	} else { //Envoie la demande à son père
		/* Créer la socket */
		int dED = socket(PF_INET, SOCK_DGRAM, 0);

		if(dED == -1){
			perror("problème création socket de envoyerDemande");
			exit(1);
		}

		printf("Processus %s: creation de la socket : ok\n", (*k).addr.sin_addr.s_addr);

		/* Nommage de la socket du destinataire (le pere)*/
    
    	/*struct sockaddr_in addr_Pere;
    	addr_Pere.sin_family=AF_INET;
    	inet_pton(AF_INET, (*k).Pere.sin_addr.s_addr, &(addr_Pere.sin_addr));
    	addr_Pere.sin_port = htons((short)(*k).Pere.port);
    	socklen_t lgA = sizeof(struct sockaddr_in);*/


		if(bind(dED, (struct sockaddr *)&k->Pere, sizeof((*k).Pere)) < 0){
			perror("Serveur : problème au bind");
			close(dED);
			exit(1);
		}

		printf("Serveur: nommage : ok\n");

		/*Envoie de la demande au père */

		char message[100];
		sprintf(message, "Je suis le jeton");

		
		int snd = sendto(dED, &message, sizeof(message), 0, (struct sockaddr*)&k->Pere, sizeof((*k).Pere)); //verif les deux derniers arguments car pas sur
		/* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
		if (snd <= 0) {
			perror("Client:pb d'envoi : ");
			close(dED); //je libère ressources avant de terminer
			exit(1); //je choisis de quitter le pgm, la suite depend de 
			// la reussite de l'envoir de la demande de connexion
		}

		(*k).Pere.sin_addr.s_addr = NULL;
		(*k).Pere.sin_port = 0;
		return 0;
	}
}

void envoyerToken(sites *k){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour
    
    /*Création de la socket d'envoi*/
    int dET = socket(PF_INET, SOCK_DGRAM, 0);
    if(dET == -1){
        perror("problème création socket de envoyerToken");
        exit(1);
    }

    printf("Processus %s : creation de la socket de envoyerToken: ok\n", (*k).addr.sin_addr.s_addr);
    
    /* Nommage de la socket du destinataire (le next)*/
    
    /*struct sockaddr_in addr_Next;
    addr_Next.sin_family=AF_INET;
    inet_pton(AF_INET, (*k).Next.sin_addr.s_addr, &(addr_Next.sin_addr));
    addr_Next.sin_port = htons((short)(*k).Next.sin_port);*/
    socklen_t lgA = sizeof(struct sockaddr_in);
    
    printf("Processus %s : J'envoi le jeton\n", (*k).addr.sin_addr.s_addr);
    
    char message[100];
    sprintf(message, "Je suis le jeton");
    
    int snd = sendto(dET, &message, sizeof(message), 0, (struct sockaddr*)&k->Next, lgA);
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton");
        close(dET); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Processus %s : Jeton envoyé \n", (*k).addr.sin_addr.s_addr);
    
    close(dET);
}

void finSC(sites* k){ //Sorti de la SC
    (*k).est_demandeur = 0;
    if ((*k).Next.sin_addr.s_addr != NULL){
        envoyerToken(k);
        (*k).jeton_present = 0;
        (*k).Next.sin_addr.s_addr = NULL;
        (*k).Next.sin_port = 0;
    }
}

void calculSC(){ //Calcul pour simuler une entrée en SC pour un site ayant le token
    //on verra apres pas important pour le moment
}

void recepReq(sockaddr_in *demandeur, sites *k){ //Comportement d'un site lors de la réception d'une requête venant du site k
    if ((*k).Pere.sin_addr.s_addr == NULL){
        if ((*k).est_demandeur == 1){
            (*k).Next.sin_addr.s_addr = (*demandeur).sin_addr.s_addr;
            (*k).Next.sin_port = (*demandeur).sin_port;
        }else{
            (*k).jeton_present = 0;
            envoyerToken(demandeur);
        }
    }else{
        envoyerDemande(demandeur);
    }
    (*k).Pere.sin_addr.s_addr = (*demandeur).sin_addr.s_addr;
    (*k).Pere.sin_port = (*demandeur).sin_port;
}

//Fonction qu'on va appeler dans le thread qui sera en attente
void reception(sites *k){ //Comportement lors de la réception du token par un site l'ayant demandé
                            //Créer une socket qui recevra le jeton
    
    /*Création de la socket de reception*/
    int dRT = socket(PF_INET, SOCK_DGRAM, 0);
    if(dRT == -1){
        perror("problème création socket de reception");
        exit(1);
    }

    printf("Processus %d : creation de la socket de reception: ok\n", (*k).addr.sin_addr.s_addr);
    
    /* Nommage de la socket du processus qui envoi le jeton*/
    struct sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);
    
    //espace memoire pour recevoir le message
    char msgRecu[100];
    
    printf("Processus %s: j'attends de recevoir un message du serveur \n", (*k).addr.sin_addr.s_addr);
    int rcv = recvfrom(dRT, &msgRecu, sizeof(msgRecu), 0, (struct sockaddr*)&addrExp, &lgAddrExp);
      
    if(rcv < 0){
        perror("Problème au niveau du recvfrom de reception");
        close(dRT);
        exit(1);
    }

    if(msgRecu == "Je suis le jeton"){
        printf("Processus %d : Jeton reçu", (*k).addr.sin_addr.s_addr);
		(*k).jeton_present = 1;
    }else if(msgRecu == "Je suis une demande") {
		printf("Processus %d : Demande reçue", (*k).addr.sin_addr.s_addr);
		sockaddr_in demandeur;
		demandeur.sin_addr.s_addr = addrExp.sin_addr.s_addr;
		demandeur.sin_port = addrExp.sin_port;
		recepDemande(&demandeur, &k);
	}else{
        printf("Erreur à la réception");
        close(dRT);
        exit(1);
    }
    close(dRT);
}

/*void recepTocken(sites *k){
	(*k).jeton_present = 1;
}*/

in_addr** connaitreIP() {
	char s[256];
	struct hostent *host;
	struct in_addr **adr;
	if (!gethostname(s, 256)){
		if ((host = gethostbyname(s)) != NULL) {
			adr = (struct in_addr **)host->h_addr_list;
    		return adr;
		}
	}
	return NULL;
}

int main(int argc, char *argv[]){

	in_addr** IP = connaitreIP();
	printf(inet_ntoa(**IP)); 	//pour afficher l'adresse IP de l'host
	sites test = init(8080);
	printf("\nIP : %s Port : %d IP du père : %s Port du père : %d IP du Next : %s Port du Next : %d \n", test.IP, test.port, test.Pere.IP, test.Pere.port, test.Next.IP, test.Next.port);

}
