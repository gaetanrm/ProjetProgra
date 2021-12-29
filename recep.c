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
    
    
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;
    
    sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(sockaddr_in);
    
    
    //espace memoire pour recevoir le message
    char msg[100];
    
    //calcul(1);
    
    
    while (args->boucleEcoute == 0){
        
        printf("\n  FONCTION RECEPTION \n");
        
        printf("\nSite %d : j'attends de recevoir un message\n", (*args->k).num);
        
        /* TCP */
        
        printf("Site %d : J'attend qu'un site se connecte à moi\n", (*args->k).num);
        
        // Je fais en sorte que ma socket d'ecoute accepte les connexions
        int dsExp = accept((*(*args).socket), (struct sockaddr*)&addrExp, &lgAddrExp);
        
        if (dsExp < 0){
            perror ("probleme accept ");
            close(dsExp);
            exit (1);
        }
    
        printf("Site %d : le site est connecté \n", (*args->k).num);

        // Je recois la demande
        int recevoirTCP = recv (dsExp, &msg, sizeof(msg), 0);
      
        if(recevoirTCP < 0){
            perror("Problème au niveau du recv de reception");
            close((*(*args).socket));
            exit(1);
        }

        printf("Site %d : J'ai bien reçu le message\n", (*args->k).num);
        
        message msgRecu;
        
        char *ptr = strtok(msg, ":");
        msgRecu.typeMessage = atoi(ptr);
        ptr = strtok(NULL, ":");
        msgRecu.demandeur.sin_addr.s_addr = inet_addr(ptr);
        ptr =  strtok(NULL, ":");
        msgRecu.demandeur.sin_port = atoi(ptr);
        
        //printf("%d", msgRecu.typeMessage);
        
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
        
        pthread_mutex_lock(&args->jeton);
        printf("Je vérouilles le mutex");
        pthread_cond_wait(&args->a_jeton, &args->jeton);
        if(msgRecu.typeMessage == 1){ //Si je reçois un jeton
            printf("Site %d : Jeton reçu du processus %s:%d\n", (*args->k).num, inet_ntoa(msgRecu.demandeur.sin_addr), ntohs(msgRecu.demandeur.sin_port));
            (*args->k).jeton_present = 1;
            pthread_cond_signal(&args->a_jeton);
            pthread_mutex_unlock(&args->jeton);
        }
        else if(msgRecu.typeMessage == 0) { //Si je reçois une demande de SC
            printf("Site %d : Demande de jeton reçue du processus %s:%d\n", (*args->k).num, inet_ntoa(msgRecu.demandeur.sin_addr), ntohs(msgRecu.demandeur.sin_port));
            calcul(2);
            recepDemande(&msgRecu, args->k, (*(*args).socket));
        }
        else{ //Si
            printf("Site %d : Le message reçu n'est ni un jeton ni une demande", (*args->k).num);
            close((*(*args).socket));
            exit(1);
        }
        close(dsExp); //Pas sur
    } 
    printf("Thread écoute terminé");
    pthread_exit(NULL);

}


        
        
        
void recepDemande(message* msg, sites *site, int sock){ //Comportement d'un site lors de la réception d'une requête venant du site k
    
    struct paramsFonctionThread * args = NULL;
    args->k = site;
    args->m = msg;
    args->socket = &sock;
    printf("\n  FONCTION RECEP DEMANDE \n");
    if ((*site).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
        (*site).Next.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*site).Next.sin_port = (*msg).demandeur.sin_port;
        
        if ((*site).est_demandeur == 1){//Si je suis tjr en SC
            printf("\n Site %d : J'ai recu une demande et je suis la racine mais je suis en SC, donc il devient mon Next\n", (*site).num);
            (*site).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
            (*site).Pere.sin_port = (*msg).demandeur.sin_port;
        }else{ //Si je ne suis pas en SC
            if (site->jeton_present == 1){ //Si j'ai le jeton je lui envoi
                (*site).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
                (*site).Pere.sin_port = (*msg).demandeur.sin_port;
                msg->typeMessage = 1;
                msg->demandeur = site->addr;
                printf("Site %d : J'ai le jeton donc je l'envoie\n", (*site).num);
                envoyerToken(site, msg, sock);
                
            }else{ //Si j'ai pas le jeton
                printf("Site %d : Il devient mon Next, mais je n'ai pas encore reçu le jeton\n", (*site).num);            }
        }
    }else{ //Si je ne suis pas la racine
        printf("Site %d : Je n'ai pas le jeton\n", (*site).num);
        envoyerDemande(args);
        (*site).Pere.sin_addr.s_addr = (*msg).demandeur.sin_addr.s_addr;
        (*site).Pere.sin_port = (*msg).demandeur.sin_port;
    }
}
