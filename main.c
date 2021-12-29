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

// TODO Quand on marque pas demande il faut pas que ça demande une SC


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
    
    printf("\n FONCTION FINSC \n");
    (*k).est_demandeur = 0;
    if ((*k).Next.sin_addr.s_addr != inet_addr("0.0.0.0")){ //Si j'ai un Next je lui envoi le jeton
        message msg;
        msg.typeMessage = 1;
        msg.demandeur = (*k).addr;
        
        printf("Site %d : J'envoi le jeton à mon next, le processus %s:%d\n", (*k).num, inet_ntoa((*k).Next.sin_addr), ntohs((*k).Next.sin_port));
        envoyerToken(k, &msg, socket);
        (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
        (*k).Next.sin_port = 0;
        
        printf("\n SUITE FONCTION FINSC \n");
        
        printf("Site %d : J'ai mis mon next à null\n", (*k).num);
    } else {
        printf("Site %d : Je n'ai pas de Next donc je garde le jeton\n", (*k).num);
    }
    
}

int main(int argc, char *argv[]){
    
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
    
    sites sommet;
    init(&sommet, port, IP_Pere, Port_Pere, i, racine);
    
    
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

    //printf("\nSite %d: creation de la socket : ok\n", i);
    
    // Passer la socket en mode ecoute
    int ect = listen(dS,10); //10 est le nb max de demande qui peuvent être mises en file d'attente
    if (ect < 0){
        printf("Site %d : je suis sourd\n", i);
        close(dS);
        exit(1);
    }
  
    //printf("Site %d: mise en écoute : ok\n", i);
    
     /* FIN TCP */
    
    
    
    /* CREATION DES THREADS, MUTEX ET CONDITION */
    
    //Pour la reception
	pthread_t threadEcoute;
	struct paramsFonctionThread tabParams;

    tabParams.k = &sommet;
    tabParams.socket = dS;
    //tabParams.idThread = i;
    //tabParams.varPartagee = &jeton;
    tabParams.boucleEcoute = 0;
    
    //Pour la demande
    pthread_t threadDemande;
    //struct paramsFonctionThread tabDem;
    
    //tabDem.k = &sommet;
    //tabDem.socket = &dS;
    
    
    
    pthread_mutex_init(&(tabParams.lock), NULL);
    pthread_cond_init(&(tabParams.a_jeton), NULL);
    
    if (pthread_create(&threadEcoute, NULL, reception, &tabParams) != 0){
        perror("Erreur création thread");
        exit(1);
    }

    printf("Site %d : Création du thread pour la réception ok\n", sommet.num);
	//Creer thread demande ici ?
    
    /* FIN CREATION THREADS */

    message msg;
    msg.demandeur = sommet.addr;
    
    tabParams.m = &msg;
    

    char arretOuDem[] = "init";
    char a[10] = "arret";
    char d[10] = "demande";
    //int ad = 2; //Position neutre
    
    do{ //Tant que je ne fais pas une demande d'arret
        printf("\n Si vous voulez faire une demande, rentrez 'demande', si vous voulez arrêter le site, rentrez 'arret' :\n");
        scanf("%s", arretOuDem);
        
        if(strcmp(arretOuDem, a) == 0){ //Si on demande un arrêt
            //ad = 0;
            printf("Vous avez demandé un arret\n");
            break;

        } else if (strcmp(arretOuDem, d) == 0) { //Si on fait une demande
            //ad = 1;
            msg.typeMessage = 0;
            printf("Type du message que j'ai avant d'envoyer la demande : %d\n", tabParams.m->typeMessage);
            printf("\nSite %d : Je fais une demande de SC\n", sommet.num);
            if (pthread_create(&threadDemande, NULL, envoyerDemande, &tabParams) != 0){ //Pas sur si je dois le mettre la ou pas
                perror("Erreur création thread");
                exit(1);
            }
            
            printf("Site %d : Création du thread pour la demande ok\n", sommet.num);
            pthread_join(threadDemande, NULL);

        } else {
            printf("Vous n'avez demandé ni un arret, ni une demande de section critique\n");
        }
        
    } while(strcmp(arretOuDem, a) != 0);

 
    
    
    //Attendre que les threads finissent
	pthread_join(threadEcoute, NULL);
    
    
    printf("Thread principal : fin du thread de reception\n");
    
    close(dS);
	pthread_exit(NULL);
	
}
