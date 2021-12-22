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

int envoyerDemande(sites* sommet, message* msg, int s){     //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
                                //resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre

    printf("\n  FONCTION ENVOYER DEMANDE \n");
    
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
        
        
        /* TCP */
        
        //Création de la socket d'envoi
        int dSPere = socket(PF_INET, SOCK_STREAM, 0);

        if(dSPere == -1){
            perror("Problème création socket du père dans envoyerDemande ");
            exit(1);
        }
        
        printf("Site %d : création de la socket de mon père ok \n", (*sommet).num);

        
        //envoyer une demande de connexion au père.
        int conn = connect(dSPere, (struct sockaddr *) &addrPere, lgAddrPere);
        
        if (conn < 0){
            perror("pb au connect dans la fonction envoyerDemande ");
            close(s);
            exit(1);
        }

        printf("Site %d : demande de connexion à mon père reussie \n", (*sommet).num);
        
        
        
        //J'envoie la taille de l'instruction
        int tailleInst = sizeof(msg); // TODO *msg ou juste msg ??
        
        ssize_t env = send(dSPere, &tailleInst,tailleInst,0);
        if (env < 1) {
            printf("Site %d : pb à l'envoi de la taille de l'instruction dans envoyerDemande\n", (*sommet).num);
            close (dSPere);
            exit (1);
        }
        
        printf("Site %d : Taille du message transmise \n", (*sommet).num);
        
        //Puis j'envoie l'instruction elle même
        env = send(dSPere,&msg,sizeof(struct message),0);
        if (env < 1) {
            printf("Site %d : pb à l'envoi de la demande\n", (*sommet).num);
            close (dSPere);
            exit (1);
        }
        
        printf("Site %d : Demande transmise\n", (*sommet).num);
        
        close(dSPere); //Pas sur
        
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
