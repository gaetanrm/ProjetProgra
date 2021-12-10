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
	char* ipPere = "127.0.1.1";
	moiMeme.IP = inet_ntoa(**connaitreIP());
	moiMeme.port = htons((short) port);
	moiMeme.Next.IP = NULL;
	moiMeme.Next.port = NULL;
	if (moiMeme.IP == ipPere) {
		moiMeme.Pere.IP = NULL;
		moiMeme.Pere.port = NULL;
		moiMeme.jeton_present = 1;
	} else {
		moiMeme.Pere.IP = ipPere; //Valeur aléatoire, faudra s'en occuper plus tard
		moiMeme.Pere.port = htons((short) 8888); //Idem
		moiMeme.jeton_present = 0;
	}
	moiMeme.est_demandeur = 0;
	moiMeme.estEn_SC = 0;
	return moiMeme;

}

int envoyerDemande(sites* k){ //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue

	(*k).est_demandeur = 1;

	if ((*k).Pere.IP == NULL){
		//appeler une méthode pour rentrer en SC
	}else{
		/* Créer une socket */
		int ds = socket(PF_INET, SOCK_DGRAM, 0);
		if(ds == -1){
			perror("Serveur : problème création socket");
			exit(1);
		}

		printf("Serveur: creation de la socket : ok\n");

		struct sockaddr_in adresse;
		adresse.sin_family=AF_INET;
		adresse.sin_addr.s_addr = INADDR_ANY;
		adresse.sin_port = htons((*k).Pere.port);

		if(bind(ds, (struct sockaddr *)&adresse, sizeof(adresse)) < 0){
			perror("Serveur : problème au bind");
			close(ds);
			exit(1);
		}

		printf("Serveur: nommage : ok\n");

		/*Envoie de la demande au père */

		long int message;
		unsigned int nbTotalOctetsEnvoyes = 0;
		unsigned int nbAppelSend = 0;
		int snd = sendto(ds, &message, sizeof(message), 0, (struct sockaddr*)&adresse, sizeof(adresse)); //verif les deux derniers arguments car pas sur
		/* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
		if (snd <= 0) {
			perror("Client:pb d'envoi : ");
			close(ds); //je libère ressources avant de terminer
			exit(1); //je choisis de quitter le pgm, la suite depend de 
			// la reussite de l'envoir de la demande de connexion
		}
		(*k).Pere.IP = NULL;
		(*k).Pere.port = NULL;
	}
}

int envoyerToken(sites *k){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour
    
    /*Création de la socket d'envoi*/
    int dET = socket(PF_INET, SOCK_DGRAM, 0);
    if(dET == -1){
        perror("problème création socket de envoyerToken");
        exit(1);
    }

    printf("Processus %s : creation de la socket de envoyerToken: ok\n", (*k).IP);
    
    /* Nommage de la socket du destinataire (le next)*/
    
    struct sockaddr_in addr_Next;
    addr_Next.sin_family=AF_INET;
    inet_pton(AF_INET, &(*k).Next.IP, &(addr_Next.sin_addr));
    addr_Next.sin_port = htons((short) atoi((*k).Next.port));
    socklen_t lgA = sizeof(struct sockaddr_in);
    
    printf("Processus %s : J'envoi le jeton\n", (*k).IP);
    
    char message[100];
    sprintf(message, "Je suis le jeton");
    
    int snd = sendto(dET, &message, sizeof(message), 0, (struct sockaddr*)&addr_Next, lgA);
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton");
        close(dET); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Processus %s : Jeton envoyé \n", (*k).IP);
    
    close(dET);
}

void finSC(sites* k){ //Sorti de la SC
    (*k).est_demandeur = 0;
    if ((*k).Next.IP != NULL){
        envoyerToken(&k);
        (*k).jeton_present = 0;
        (*k).Next.IP = NULL;
        (*k).Next.port = NULL;
    }
}

void calculSC(){ //Calcul pour simuler une entrée en SC pour un site ayant le token
    //on verra apres pas important pour le moment
}

void recepDemande(parent *envoyeur, sites *receveur){ //Comportement d'un site lors de la réception d'une requête venant du site k
    if ((*receveur).Pere.IP == NULL){
        if ((*receveur).est_demandeur == 1){
            (*receveur).Next.IP = (*envoyeur).IP;
            (*receveur).Next.port = (*envoyeur).port;
        }else{
            (*receveur).jeton_present = 0;
            envoyerToken(&envoyeur);
        }
    }else{
        envoyerDemande(&envoyeur);
    }
    (*receveur).Pere.IP = (*envoyeur).IP;
    (*receveur).Pere.port = (*envoyeur).port;
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

    printf("Processus %d : creation de la socket de reception: ok\n", (*k).IP);
    
    /* Nommage de la socket du processus qui envoi le jeton*/
    struct sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);
    
    //espace memoire pour recevoir le message
    char msgRecu[100];
    
    printf("Processus %d: j'attends de recevoir un message du serveur \n", (*k).IP);
    int rcv = recvfrom(dRT, &msgRecu, sizeof(msgRecu), 0, (struct sockaddr*)&addrExp, &lgAddrExp);
      
    if(rcv < 0){
        perror("Problème au niveau du recvfrom de reception");
        close(dRT);
        exit(1);
    }

    if(msgRecu == "Je suis le jeton"){
        printf("Processus %d : Jeton reçu", (*k).IP);
		(*k).jeton_present = 1;
    }else if(msgRecu == "Je suis une demande") {
		printf("Processus %d : Demande reçue", (*k).IP);
		parent demandeur;
		demandeur.IP = addrExp.sin_addr.s_addr;
		demandeur.port = addrExp.sin_port;
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
	if (!gethostname(s, 256))
		if ((host = gethostbyname(s)) != NULL) {
			adr = (struct in_addr **)host->h_addr_list;
    		return adr;
		}
}

int main(int argc, char *argv[]){

	in_addr** IP = connaitreIP();
	printf(inet_ntoa(**IP)); 	//pour afficher l'adresse IP de l'host
	sites test = init(8080);
	printf("\nIP : %s Port : %d IP du père : %s Port du père : %d IP du Next : %s Port du Next : %d \n", test.IP, test.port, test.Pere.IP, test.Pere.port, test.Next.IP, test.Next.port);

	
	/*
	// je passe en paramètre un numero de port pour la socket du serveur
  
  	if (argc<2){
    	printf("utilisation: %s numero_port\n", argv[0]);
    	exit(1);
  	}

  	// Créer une socket 
 
  	int ds = socket(PF_INET, SOCK_DGRAM, 0);
  	if(ds == -1){
  		perror("Serveur : problème création socket");
  		exit(1);
  	}

 	 printf("Serveur: creation de la socket : ok\n");

  	struct sockaddr_in adresse;
  	adresse.sin_family=AF_INET;
  	adresse.sin_addr.s_addr = INADDR_ANY;
  	adresse.sin_port = htons((short) atoi(argv[1]));

 	if(bind(ds, (struct sockaddr *)&adresse, sizeof(adresse)) < 0){
  		perror("Serveur : problème au bind");
  		close(ds);
  		exit(1);
  	}

  	printf("Serveur: nommage : ok\n");
	  */
}
