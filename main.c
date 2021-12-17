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
#include "calcul.h"


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


int envoyerDemande(sites* sommet, message* msg, int socket){ 	//Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
								//resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre 

    if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){
        printf("Je deviens demandeur\n");
        (*sommet).est_demandeur = 1;
    }

	if ((*sommet).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
		//FAUX (*k).estEn_SC = 1;
		return 1;
	} else { //Envoie la demande à son père
        sockaddr_in addrPere = (*sommet).Pere;
        socklen_t lgAddrPere = sizeof(sockaddr_in);
        printf("J'envoie la demande à mon père\n");
		int snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&addrPere, lgAddrPere);
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


void envoyerToken(sites *k, message* msg, int socket){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour

    printf("Processus %d : J'envoi le jeton\n", (*k).num);
    
    int snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&k->Next, sizeof(struct sockaddr_in));    //SI pas de next, erreur a l'éxécution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
    (*k).Next.sin_port = 0;

    if (snd <= 0) {
        perror("pb d'envoi du jeton\n");
        close(socket); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Processus %u : Jeton envoyé \n", (*k).num);
    
    (*k).jeton_present = 0;
}

void finSC(sites* k, int socket){ //Sorti de la SC
    (*k).est_demandeur = 0;
    if ((*k).Next.sin_addr.s_addr != inet_addr("0.0.0.0")){
        message msg;
        msg.typeMessage = 1;
        msg.demandeur = (*k).addr;
        envoyerToken(k, &msg, socket);
        (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
        (*k).Next.sin_port = 0;
    }
}


void recepDemande(message* msg, sites *k, int socket){ //Comportement d'un site lors de la réception d'une requête venant du site k
    if ((*k).Pere.sin_addr.s_addr == 0){
        (*k).Next.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*k).Next.sin_port = (*msg).demandeur.sin_port;
        if ((*k).est_demandeur == 1){
            printf("J'ai recu une demande et je suis la racine mais je suis deja demandeur, donc il devient mon Next\n");
        }else{
            if (k->jeton_present == 1){
                printf("J'envois le token\n");
                (*k).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
                (*k).Pere.sin_port = (*msg).demandeur.sin_port;
                msg->typeMessage = 1;
                msg->demandeur = k->addr;
                envoyerToken(k, msg, socket);
            }else{
                printf("Il devient mon Next, mais je n'ai pas encore reçu le jeton\n");
            }
        }
    }else{
        printf("Je renvoie la demande\n");
        envoyerDemande(k, msg, socket);
        (*k).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*k).Pere.sin_port = (*msg).demandeur.sin_port;
    }
}

//Fonction qu'on va appeler dans le thread qui sera en attente
void * reception(void * params){ //Comportement lors de la réception du token par un site l'ayant demandé
                            //Créer une socket qui recevra le jeton
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;

    sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);

    //espace memoire pour recevoir le message
    message msg;
    calcul(1);
    
    printf("\nProcessus %u: j'attends de recevoir un message du serveur \n", (*args->k).addr.sin_addr.s_addr);

    while ((*args).boucleEcoute == 0){

        int rcv = recvfrom((*args).socket, &msg, sizeof(struct message), 0, (struct sockaddr*)&addrExp, &lgAddrExp);

        if(rcv < 0){
            perror("Problème au niveau du recvfrom de reception");
            close((*args).socket);
            exit(1);
        }

        printf("L'expéditeur : %u\n", msg.demandeur.sin_port);

        if(msg.typeMessage == 1){
            printf("Processus %u : Jeton reçu\n", (*args->k).num);
            (*args->k).jeton_present = 1;
            calcul(2);
            printf("J'ai terminé ma Section Critique\n");
            finSC(args->k, args->socket);
        }
        else if(msg.typeMessage == 0) {
            printf("Processus %u : Demande reçue\n", (*args->k).num);
            recepDemande(&msg, args->k, (*args).socket);
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
    
    
    
    /* Création de la socket*/
    int dS = socket(PF_INET, SOCK_DGRAM, 0);

    if(dS == -1){
        perror("problème création socket du sommet");
        exit(1);
    }

    if (bind(dS, (const struct sockaddr *)&sommet, sizeof(sommet)) < 0){ 
        perror("Erreur bind ecoute: ");
        exit(1);
    }

    printf("Sommet %d: creation de la socket : ok\n", i);

	pthread_t threadEcoute;

	struct paramsFonctionThread tabParams;

	/*struct predicatRdv jeton;
	pthread_mutex_init(&(jeton.lock), NULL);
	pthread_cond_init(&(jeton.have_jeton), NULL);*/

	tabParams.k = &sommet;
	tabParams.socket = dS;
	tabParams.idThread = i;
	//tabParams.varPartagee = &jeton;
    tabParams.boucleEcoute = 0;

	if (pthread_create(&threadEcoute, NULL, reception, &tabParams) != 0){
		perror("Erreur création thread");
		exit(1);
	}

    message msg;
    int userReady = 1;

    while (userReady != 0){ 

        printf("Tapez 0 si vous voulez arreter le site, et 1 pour envoyer une demande\n");
        scanf("%d", &userReady);

        if (userReady == 0){
            tabParams.boucleEcoute = 1;
        }else if (userReady == 1){
            msg.typeMessage = 0;
            msg.demandeur = sommet.addr;

            if(envoyerDemande(&sommet, &msg, dS) == 1){ //Je suis la racine
                printf("Je suis la racine donc je rentre en SC\n");
                calcul(2);
                printf("Fin de ma Section Critique \n");
                finSC(&sommet, dS);
            } else {
                printf("J'ai envoyé la demande à mon père\n");
            }
        }else{
            printf("Erreur, veuillez réessayer !\n");
        }
    } //Pour arrêter le thread au bout de 20 sec d'inactivité.

	pthread_join(threadEcoute, NULL);

	printf("Thread principal : fin du thread d'écoute\n");
	//je termine proprement !
	pthread_exit(NULL);
	close(dS);
}
