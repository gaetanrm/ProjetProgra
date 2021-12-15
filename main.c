#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "main.h"


sites init(int port, in_addr IP_p, int Port_p, int num, int rac){ //Initialisation de tous les sites au démarrage de l'algo
	//Chacun doit envoyer son num aux autres pour savoir qui est le 1, donc qui sera la racine au départ de l'algo même si il n'a pas fait de demande.
	//C'est juste pour que l'algo puisse fonctionner ensuite car il a besoin d'une racine pour cela
    
	sites sommet;
    
    sommet.num = num;
    
    //addr
    sommet.addr.sin_family = AF_INET;
    sommet.addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //TODO à ne pas mettre en dur
    sommet.addr.sin_port = htons((short)port);
    printf("IP du sommet: %s\n", inet_ntoa(sommet.addr.sin_addr));
    printf("Port du sommet: %hu\n", ntohs(sommet.addr.sin_port));
	
    //Next
    sommet.Next.sin_family = AF_INET;
    sommet.Next.sin_addr.s_addr = inet_addr("0.0.0.0"); //Equivalent à NULL
    sommet.Next.sin_port = 0;
    printf("IP du Next: %s\n", inet_ntoa(sommet.Next.sin_addr));
    printf("Port du Next: %hu\n", ntohs(sommet.Next.sin_port));
	
    //Pere
    sommet.Pere.sin_family = AF_INET;
    
	if (sommet.num != rac) {
		sommet.Pere.sin_addr = IP_p;
        printf("IP du père: %s\n", inet_ntoa(sommet.Pere.sin_addr));
		sommet.Pere.sin_port = htons((short)Port_p);
        printf("Port du Pèret: %hu\n", ntohs(sommet.Pere.sin_port));
		sommet.jeton_present = 0;
	} else {
		sommet.Pere.sin_addr.s_addr = inet_addr("0.0.0.0"); //Valeur aléatoire, faudra s'en occuper plus tard
		sommet.Pere.sin_port = 0; //Idem
        printf("IP du père: %s\n", inet_ntoa(sommet.Pere.sin_addr));
        printf("Port du Père: %hu\n", ntohs(sommet.Pere.sin_port));
		sommet.jeton_present = 1;
	}
    
	sommet.est_demandeur = 0;
	sommet.estEn_SC = 0;
    
    printf("\nSite %d initialisé\n", sommet.num);
    //printf("\nIP : %s \nPort : %d \nIP du père : %s \nPort du père : %d \nIP du Next : %u \nPort du Next : %d \nJeton présent ? %d\n", inet_ntoa(sommet.addr.sin_addr), ntohs(sommet.addr.sin_port), inet_ntoa(sommet.Pere.sin_addr), ntohs(sommet.Pere.sin_port), sommet.Next.sin_addr.s_addr, sommet.Next.sin_port, sommet.jeton_present);
    
    return sommet;
}

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


int envoyerDemande(sites* sommet, int socket){ 	//Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
								//resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre
	(*sommet).est_demandeur = 1;

	if ((*sommet).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
		//FAUX (*k).estEn_SC = 1;
		return 1;
	} else { //Envoie la demande à son père

		char message[100];
		sprintf(message, "Je suis le jeton");
		
		ssize_t snd = sendto(socket, &message, sizeof(message), 0, (struct sockaddr*)&sommet->Pere, sizeof(struct sockaddr_in));
		/* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
		if (snd <= 0) {
			perror("Client:pb d'envoi : ");
			close(socket);
			exit(1);
		}

		(*sommet).Pere.sin_addr.s_addr = 0;
		(*sommet).Pere.sin_port = 0;
		return 0;
	}
}


void envoyerToken(sites *k, int socket){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour

    printf("Processus %d : J'envoi le jeton\n", (*k).num);
    
    char message[100];
    sprintf(message, "Je suis le jeton");
    
    long int snd = sendto(socket, &message, sizeof(message), 0, (struct sockaddr*)&k->Next, sizeof(struct sockaddr_in));    //SI pas de next, erreur a l'éxécution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton");
        close(socket); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Processus %u : Jeton envoyé \n", (*k).addr.sin_addr.s_addr);
    
    (*k).jeton_present = 0;
}

void finSC(sites* k, int socket){ //Sorti de la SC
    (*k).est_demandeur = 0;
    if ((*k).Next.sin_addr.s_addr != inet_addr("0.0.0.0")){
        envoyerToken(k, socket);
        (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
        (*k).Next.sin_port = 0;
    }
}

void calculSC(){ //Calcul pour simuler une entrée en SC pour un site ayant le token
    //on verra apres pas important pour le moment
}


void recepDemande(sites *demandeur, sites *k, int socket){ //Comportement d'un site lors de la réception d'une requête venant du site k
    if ((*k).Pere.sin_addr.s_addr == 0){
        if ((*k).est_demandeur == 1){
            (*k).Next.sin_addr.s_addr = (*demandeur).addr.sin_addr.s_addr;
            (*k).Next.sin_port = (*demandeur).addr.sin_port;
        }else{
            envoyerToken(k, socket);
        }
    }else{
        envoyerDemande(demandeur, socket);
    }
    (*k).Pere.sin_addr.s_addr = (*demandeur).addr.sin_addr.s_addr;
    (*k).Pere.sin_port = (*demandeur).addr.sin_port;
}

//Fonction qu'on va appeler dans le thread qui sera en attente
void * reception(void * params){ //Comportement lors de la réception du token par un site l'ayant demandé
                            //Créer une socket qui recevra le jeton
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;

    struct sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);

    //espace memoire pour recevoir le message
    char* msgRecu;
    
    printf("\nProcessus %u: j'attends de recevoir un message du serveur \n", (*args->k).addr.sin_addr.s_addr);

    while ((*args).boucleEcoute == 0){

        int rcv = recvfrom((*args).socket, &msgRecu, sizeof(msgRecu), 0, (struct sockaddr*)&addrExp, &lgAddrExp);
      
        if(rcv < 0){
            perror("Problème au niveau du recvfrom de reception");
            close((*args).socket);
            exit(1);
        }

        if(strcmp(msgRecu,"Je suis le jeton")){
            printf("Processus %d : Jeton reçu", (*args->k).addr.sin_addr.s_addr);
            (*args->k).jeton_present = 1;
        }
        else if(strcmp(msgRecu,"Je suis une demande")) {
            printf("Processus %d : Demande reçue", (*args->k).addr.sin_addr.s_addr);
            sites demandeur = (*args->k);
            demandeur.addr.sin_addr.s_addr = addrExp.sin_addr.s_addr;
            demandeur.addr.sin_port = addrExp.sin_port;
            recepDemande(&demandeur, args->k, (*args).socket);
        }
        else{
            printf("Erreur à la réception");
            close((*args).socket);
            exit(1);
        }
    }
    pthread_exit(NULL);

}

/*void recepTocken(sites *k){
	(*k).jeton_present = 1;
}*/

int main(int argc, char *argv[]){

    //Passer en paramètre le père du site
    
    if (argc != 6){
        printf("utilisation : %s num_processus num_racine Port IP_Pere Port_Pere \n", argv[0]);
        exit(0);
    }
    
    
    //Numero du processus
    int i = atoi(argv[1]);
    
    int racine = atoi(argv[2]);
    
    int port = atoi(argv[3]);

    struct in_addr IP_Pere;
    IP_Pere.s_addr = inet_addr(argv[4]);
    
    int Port_Pere = atoi(argv[5]);
    
    sites sommet = init(port, IP_Pere, Port_Pere, i, racine);
    
    printf("\nSite %d initialisé\n", sommet.num);
    
    
    
    /* Création de la socket*/
    int dS = socket(PF_INET, SOCK_DGRAM, 0);

    if(dS == -1){
        perror("problème création socket du sommet");
        exit(1);
    }

    printf("Sommet %d: creation de la socket : ok\n", i);
    
    if(envoyerDemande(&sommet, dS) == 1){ //Je suis la racine
        printf("Je suis la racine donc je rentre en SC");
        
    } else {
        printf("J'ai envoyé la demande à mon père");
    }

	pthread_t threadEcoute;

	struct paramsFonctionThread tabParams;

	struct predicatRdv jeton;
	pthread_mutex_init(&(jeton.lock), NULL);
	pthread_cond_init(&(jeton.have_jeton), NULL);
	jeton.nbSitesAvecToken = 1;

	tabParams.k = &sommet;
	tabParams.socket = dS;
	tabParams.idThread = i;
	tabParams.varPartagee = &jeton;
    tabParams.boucleEcoute = 0;

	if (pthread_create(&threadEcoute, NULL, reception, &tabParams) != 0){
		perror("Erreur création thread");
		exit(1);
	}

	pthread_join(threadEcoute, NULL);

	printf("Thread principal : fin du thread d'écoute\n");
	//je termine proprement !
	pthread_exit(NULL);
	close(dS);
}
