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
#include "envDem.h"
#include "calcul.h"

int envoyerDemande(sites* sommet, message* msg, int s){     //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
                                //resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre

    printf("\n  FONCTION ENVOYER DEMANDE \n");
    
    if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
        printf("Site %d : Je suis dans la fonction envoyerDemande : je deviens demandeur\n", (*sommet).num);
        (*sommet).est_demandeur = 1;
    }

    if ((*sommet).Pere.sin_addr.s_addr == inet_addr("0.0.0.0") && (*sommet).jeton_present == 1){ //Si je suis la racine et que j'ai le jeton
        //Je rentre directement en SC
        
        printf("Site %d : Je rentre en section critique", (*sommet).num);
        calcul(7);
        
        printf("Pour sortir de la SC tapez 1 : ");
        int fSC = 0;
        scanf("%d", &fSC);
        while (fSC != 1) {
            int i = 0;
            i++;
        }
        
        printf("Site %d : J'ai terminé ma Section Critique\n", (*sommet).num);
        
        finSC(sommet, s);
        
        return 1;
        
    } else { //Envoie la demande à son père
        printf("Site %d : J'envoi la demande à mon père, le processus %s:%d\n", (*sommet).num, inet_ntoa((*sommet).Pere.sin_addr), ntohs((*sommet).Pere.sin_port));

        
        /* TCP */
        
        //Création de la socket d'envoi
        int dSPere = socket(PF_INET, SOCK_STREAM, 0);

        if(dSPere == -1){
            perror("Problème création socket du père dans envoyerDemande ");
            exit(1);
        }
        
        printf("Site %d : création de la socket pour communiquer avec mon père ok \n", (*sommet).num);
        
        sockaddr_in addrPere;
        addrPere.sin_addr = (*sommet).Pere.sin_addr;
        addrPere.sin_port = (*sommet).Pere.sin_port;
        addrPere.sin_family = AF_INET;
        socklen_t lgAddrPere = sizeof(struct sockaddr_in);

        
        //envoyer une demande de connexion au père.
        int conn = connect(dSPere, (struct sockaddr *) &addrPere, lgAddrPere);
        
        if (conn < 0){
            perror("pb au connect dans la fonction envoyerDemande ");
            close(s);
            exit(1);
        }

        printf("Site %d : Demande de connexion à mon père reussie, je suis connecté au site %s:%d \n", (*sommet).num, inet_ntoa(addrPere.sin_addr), ntohs(addrPere.sin_port));
        
        
        char message[100];
        snprintf(message, 100, "%d:%s:%d:", msg->typeMessage, inet_ntoa(msg->demandeur.sin_addr), msg->demandeur.sin_port);
        
        //Puis j'envoie l'instruction elle même
        ssize_t env = send(dSPere, &message, sizeof(struct message),0);
        if (env < 1) {
            printf("Site %d : pb à l'envoi de la demande\n", (*sommet).num);
            close (dSPere);
            exit (1);
        }
        
        printf("Site %d : Demande transmise à mon père \n", (*sommet).num);
        
        //close(dSPere); //Pas sur
        
        /* FIN TCP */
        
        /* UDP
         
        ssize_t snd = sendto(s, msg, sizeof(struct message), 0, (struct sockaddr*)&addrPere, lgAddrPere);
        
        if (snd <= 0) {
            perror("Client:pb d'envoi : ");
            close(s);
            exit(1);
        }
        
        FIN UDP */

        if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
            printf("Site %d : Je met mon père à null car j'ai envoyé une demande donc je me concidère comme la racine\n", (*sommet).num);
            (*sommet).Pere.sin_addr.s_addr = inet_addr("0.0.0.0");
            (*sommet).Pere.sin_port = 0;
        }
        
        return 0;
    }
}
