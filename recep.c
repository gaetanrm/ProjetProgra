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
#include "recep.h"
#include "calcul.h"

//Fonction qu'on va appeler dans le thread qui sera en attente
void * reception(void * params){ //Comportement lors de la réception du token par un site l'ayant demandé
                            //Créer une socket qui recevra le jeton
    
    printf("\n  FONCTION RECEPTION \n");
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;

    sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);
    
    
    /* TCP */
    
    // Je fais en sorte que ma socket d'ecoute accepte les connexions
    int dsExp = accept((*args).socket, (struct sockaddr*)&addrExp, &lgAddrExp);
    
    if (dsExp < 0){
        perror ("probleme accept ");
        close(dsExp);
        exit (1);
    }
    
    printf("Site %d : le processus %s:%d est connecté \n", (*args->k).num, inet_ntoa(addrExp.sin_addr), ntohs(addrExp.sin_port)); //TODO arguments à vérifier
    
    /* FIN TCP */
    
    
    //espace memoire pour recevoir le message
    message msg;
    calcul(1);
    printf("\nSite %d : j'attends de recevoir un message \n", (*args->k).num);
    
    
    while (args->boucleEcoute == 0){
        
        /*TCP */
        
        // Je recois la taille de l'instruction
        int tailleInst;
        ssize_t recevoirTCP = recv (dsExp, &tailleInst, sizeof(tailleInst),0);
      
        if(recevoirTCP < 0){
            perror("pb à la réception de la taille de l'instruction ");
            close((*args).socket);
            exit(1);
          }

         printf("Site %d : J'ai bien reçu la taille de l'instruction : %d \n", (*args->k).num, tailleInst);

        // Puis l'instruction elle même
        recevoirTCP = recv (dsExp, &msg, sizeof(struct message), 0);
      
         if(recevoirTCP < 0){
              perror("Problème au niveau du 2ieme recv de reception");
              close((*args).socket);
               exit(1);
          }

         printf("Site %d : J'ai bien reçu le message\n", (*args->k).num);
        
        close(dsExp); //Pas sur
        
        /* FIN TCP */


        /* UDP
        ssize_t rcv = recvfrom((*args).socket, &msg, sizeof(struct message), 0, (struct sockaddr*)&addrExp, &lgAddrExp);

        if(rcv < 0){
            perror("Problème au niveau du recvfrom de reception");
            close((*args).socket);
            exit(1);
        }

        //printf("L'expéditeur : %u\n", ntohs(msg.demandeur.sin_port));
         
         FIN UDP */

        if(msg.typeMessage == 1){
            printf("Site %d : Jeton reçu du processus %s:%d\n", (*args->k).num, inet_ntoa(msg.demandeur.sin_addr), ntohs(msg.demandeur.sin_port));
            (*args->k).jeton_present = 1;
            printf("Site %d : Je rentre en section critique", (*args->k).num);
            calcul(7);
            printf("Pour sortir de la SC tapez 1 : ");
            int fSC = 0;
            scanf("%d", &fSC);
            while (fSC != 1) {
                int i = 0;
                i++;
            }
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


void recepDemande(message* msg, sites *k, int socket){ //Comportement d'un site lors de la réception d'une requête venant du site k
    
    printf("\n  FONCTION RECEP DEMANDE \n");
    if ((*k).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
        (*k).Next.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*k).Next.sin_port = (*msg).demandeur.sin_port;
        
        if ((*k).est_demandeur == 1){//Si je suis tjr en SC
            printf("\n Site %d : J'ai recu une demande et je suis la racine mais je suis en SC, donc il devient mon Next\n", (*k).num);
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
