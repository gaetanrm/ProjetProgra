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
//#include "calcul.h"

void calcul(int a){

    for(int i=0; i<a; i++){
        int sum;
        for (int i = 0; i<10000; i++) {
            sum = i*4000;
        }
        printf("...\n");
    }
}


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

    if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
        printf("Site %d : Je suis dans la fonction envoyerDemande : je deviens demandeur\n", (*sommet).num);
        (*sommet).est_demandeur = 1;
    }

	if ((*sommet).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
		return 1;
        
	} else { //Envoie la demande à son père
        printf("Site %d : J'envoi la demande à mon père, le processus %s:%d\n", (*sommet).num, inet_ntoa((*sommet).Pere.sin_addr), ntohs((*sommet).Pere.sin_port));
        sockaddr_in addrPere = (*sommet).Pere;
        socklen_t lgAddrPere = sizeof(sockaddr_in);
        
		ssize_t snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&addrPere, lgAddrPere);
		/* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
		if (snd <= 0) {
			perror("Client:pb d'envoi : ");
			close(socket);
			exit(1);
		}

        if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
            printf("Site %d : Je met mon père à null car j'ai envoyé une demande donc je me concidère comme la racine\n", (*sommet).num);
            (*sommet).Pere.sin_addr.s_addr = inet_addr("0.0.0.0");
            (*sommet).Pere.sin_port = 0;
        }
        
		return 0;
	}
}


void envoyerToken(sites *k, message* msg, int socket){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour

    printf("Processus %d : J'envoi le jeton\n", (*k).num);
    
    int snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&k->Next, sizeof(struct sockaddr_in));    //SI pas de next, erreur a l'éxécution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton\n");
        close(socket); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Site %d : Jeton envoyé au processus %s:%d\n", (*k).num, inet_ntoa((*k).Next.sin_addr), ntohs((*k).Next.sin_port));
    
    //J'ai envoyé le jeton à mon Next donc je met mon Next à null
    (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
    (*k).Next.sin_port = 0;
    
    (*k).jeton_present = 0;
}

void finSC(sites* k, int socket){ //Sorti de la SC
    (*k).est_demandeur = 0;
    if ((*k).Next.sin_addr.s_addr != inet_addr("0.0.0.0")){ //Si j'ai un Next je lui envoi le jeton
        message msg;
        msg.typeMessage = 1;
        msg.demandeur = (*k).addr;
        
        printf("J'envoi le jeton à mon next, le processus %s:%d\n", inet_ntoa((*k).Next.sin_addr), ntohs((*k).Next.sin_port));
        envoyerToken(k, &msg, socket);
        (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
        (*k).Next.sin_port = 0;
    }
}


void recepDemande(message* msg, sites *k, int socket){ //Comportement d'un site lors de la réception d'une requête venant du site k
    if ((*k).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
        (*k).Next.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*k).Next.sin_port = (*msg).demandeur.sin_port;
        
        if ((*k).est_demandeur == 1){//Si je suis tjr en SC
            printf("J'ai recu une demande et je suis la racine mais je suis en SC, donc il devient mon Next\n");
            (*k).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
            (*k).Pere.sin_port = (*msg).demandeur.sin_port;
        }else{ //Si je ne suis pas en SC
            if (k->jeton_present == 1){ //Si j'ai le jeton je lui envoi
                (*k).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
                (*k).Pere.sin_port = (*msg).demandeur.sin_port;
                msg->typeMessage = 1;
                msg->demandeur = k->addr;
                envoyerToken(k, msg, socket);
                
            }else{ //Si j'ai pas le jeton
                printf("Site %d : Il devient mon Next, mais je n'ai pas encore reçu le jeton\n", (*k).num);            }
        }
    }else{ //Si je ne suis pas la racine
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
    printf("\nSite %d : j'attends de recevoir un message \n", (*args->k).num);
    
    while (args->boucleEcoute == 0){

        ssize_t rcv = recvfrom((*args).socket, &msg, sizeof(struct message), 0, (struct sockaddr*)&addrExp, &lgAddrExp);

        if(rcv < 0){
            perror("Problème au niveau du recvfrom de reception");
            close((*args).socket);
            exit(1);
        }

        //printf("L'expéditeur : %u\n", ntohs(msg.demandeur.sin_port));

        if(msg.typeMessage == 1){
            printf("Site %d : Jeton reçu du processus %s:%d\n", (*args->k).num, inet_ntoa(msg.demandeur.sin_addr), ntohs(msg.demandeur.sin_port));
            (*args->k).jeton_present = 1;
            printf("Site %d : Je rentre en section critique", (*args->k).num);
            calcul(7);
            printf("Site %d : J'ai terminé ma Section Critique\n", (*args->k).num);
            finSC(args->k, args->socket);
        }
        else if(msg.typeMessage == 0) {
            printf("Site %d : Demande reçue du processus %s:%d\n", (*args->k).num, inet_ntoa(msg.demandeur.sin_addr), ntohs(msg.demandeur.sin_port));
            calcul(2);
            recepDemande(&msg, args->k, (*args).socket);
        }
        else{
            printf("Erreur à la réception");
            close((*args).socket);
            exit(1);
        }
    }
    printf("Thread écoute terminé");
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

    if (bind(dS, (const struct sockaddr *)&sommet, sizeof(struct sockaddr)) < 0){ 
        perror("Erreur bind ecoute ");
        exit(1);
    }

    printf("Sommet %d: creation de la socket : ok\n", i);

	pthread_t threadEcoute;

	struct paramsFonctionThread tabParams;

	/*struct predicatRdv jeton;
	pthread_mutex_init(&(jeton.lock), NULL);
	pthread_cond_init(&(jeton.have_jeton), NULL);*/

    int tempsAlgo;
    printf("Rentrez le nombre de demande que vous souhaitez pour ce site:\n");
    scanf("%d", &tempsAlgo);
    calcul(3);
    
	tabParams.k = &sommet;
	tabParams.socket = dS;
	tabParams.idThread = i;
	//tabParams.varPartagee = &jeton;
    tabParams.boucleEcoute = 0;

	if (pthread_create(&threadEcoute, NULL, reception, &tabParams) != 0){
		perror("Erreur création thread");
		exit(1);
	}

    printf("Site %d : Création du thread pour la réception ok\n", sommet.num);

    message msg;
    /*int userReady = 1; //Si on veut choisir quand chaque site fait une demande.

    while (tabParams.boucleEcoute == 0){ 

        calcul(2);
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
    }*/

    for (int i = 0; i < tempsAlgo; i++){
        msg.typeMessage = 0;
        msg.demandeur = sommet.addr;

        if(envoyerDemande(&sommet, &msg, dS) == 1){ //Je suis la racine
            while(sommet.jeton_present == 0){
                printf("Je suis la racine mais je n'ai pas encore le jeton donc j'attends\n");
            }
            if (sommet.jeton_present == 1){ //Si j'ai le jeton alors je rentre en SC
                printf("Je suis la racine donc je rentre en SC\n");
                calcul(2);
                printf("Fin de ma Section Critique \n");
                finSC(&sommet, dS);
            }/*else{
                printf("je suis la racine mais je n'ai pas encore le token donc j'attends");
                sommet.Next.sin_addr.s_addr = sommet.addr.sin_addr.s_addr;
                sommet.Next.sin_port = sommet.addr.sin_port;
                calcul(2);
            }
        } else {
            printf("J'ai envoyé la demande à mon père\n");*/
        }
    }

	pthread_join(threadEcoute, NULL);

	printf("Thread principal : fin du thread d'écoute\n");
	//je termine proprement !
    close(dS);
	pthread_exit(NULL);
	
}
