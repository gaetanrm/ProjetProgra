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
#include "initialisation.h"
#include "envDem.h"
#include "calcul.h"
#include "envJeton.h"
#include "recep.h"

// TODO A la fin faire en sorte d'afficher qui est son père pou r avoir une idée de l'architecture finale

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
    
    
    /* UDP
     
    // Création de la socket
    int dS = socket(PF_INET, SOCK_DGRAM, 0);

    if(dS == -1){
        perror("problème création socket du sommet");
        exit(1);
    }

    if (bind(dS, (const struct sockaddr *)&sommet, sizeof(struct sockaddr)) < 0){ 
        perror("Erreur bind ecoute ");
        exit(1);
    }
    

    //printf("Sommet %d: creation de la socket UDP : ok\n", i);
    
    FIN UDP */
    
    
    /* TCP */
    
    // Création de la socket d'ecoute pour TCP
    int dS = socket(PF_INET, SOCK_STREAM, 0);

    if(dS == -1){
        perror("problème création socket du sommet ");
        exit(1);
    }

    if (bind(dS, (const struct sockaddr *)&sommet, sizeof(struct sockaddr)) < 0){
        perror("Erreur bind ecoute ");
        exit(1);
    }

    printf("\nSite %d: creation de la socket : ok\n", i);
    
    // Passer la socket en mode ecoute
    
    int ecoute = listen(dS,10); //10 est le nb max de demande qui peuvent être mises en file d'attente
    if (ecoute < 0){
        printf("Site %d : je suis sourd\n", i);
        close(dS);
        exit(1);
    }
  
    printf("Site %d: mise en écoute : ok\n", i);
    
    /* FIN TCP */
    

	pthread_t threadEcoute;

	struct paramsFonctionThread tabParams;

	/*struct predicatRdv jeton;
	pthread_mutex_init(&(jeton.lock), NULL);
	pthread_cond_init(&(jeton.have_jeton), NULL);*/

    int tempsAlgo;
    printf("Rentrez le nombre de demande que vous souhaitez pour ce site :\n");
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

    //printf("Site %d : Création du thread pour la réception ok\n", sommet.num);

    message msg;
    msg.typeMessage = 0;
    msg.demandeur = sommet.addr;
    
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
        /* Pk on met ça dans la boucle ?
        msg.typeMessage = 0;
        msg.demandeur = sommet.addr; */

        if (envoyerDemande(&sommet, &msg, dS) == 1){ //Je suis la racine
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
