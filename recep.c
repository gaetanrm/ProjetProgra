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

/* Fonction qu'on va appeler dans le thread qui sera en attente*/

void * reception(void * params){
    
    
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;
    
    sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(sockaddr_in);
    
    
    //espace memoire pour recevoir le message
    char msg[100];
    
    
    while (1){
        
        printf("\n  FONCTION RECEPTION \n");
        
        printf("\nSite %d : J'attends de recevoir un message\n", args->k->num);
        
        /* TCP */
        
        printf("Site %d : J'attend qu'un site se connecte à moi\n", args->k->num);
        
        // Je fais en sorte que ma socket d'ecoute accepte les connexions
        int dsExp = accept((*args).socket, (struct sockaddr*)&addrExp, &lgAddrExp);
        
        if (dsExp < 0){
            perror ("probleme accept ");
            close(dsExp);
            exit (1);
        }
    
        printf("Site %d : Un site s'est connecté \n", args->k->num);

        // Je recois la demande
        int recevoirTCP = recv (dsExp, &msg, sizeof(msg), 0);
      
        if(recevoirTCP < 0){
            perror("Problème au niveau du recv de reception");
            close((*args).socket);
            exit(1);
        }

        printf("Site %d : J'ai bien reçu un message\n", args->k->num);
        
        message msgRecu;
        
        char *ptr = strtok(msg, ":");
        msgRecu.typeMessage = atoi(ptr);
        ptr = strtok(NULL, ":");
        msgRecu.demandeur.sin_addr.s_addr = inet_addr(ptr);
        ptr = strtok(NULL, ":");
        msgRecu.demandeur.sin_port = atoi(ptr);
        
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
        
        
        if(msgRecu.typeMessage == 1){ //Si je reçois un jeton
            printf("\nSite %d : J'ai reçu un jeton du processus %s:%d\n", args->k->num, inet_ntoa(msgRecu.demandeur.sin_addr), ntohs(msgRecu.demandeur.sin_port));
            pthread_mutex_lock(&args->lock);
            args->k->jeton_present = 1;
            pthread_cond_signal(&args->a_jeton);
            pthread_mutex_unlock(&args->lock);
        }
        else if(msgRecu.typeMessage == 0) { //Si je reçois une demande de SC
            printf("\nSite %d : Demande de jeton reçu du site %s:%d\n", args->k->num, inet_ntoa(msgRecu.demandeur.sin_addr), ntohs(msgRecu.demandeur.sin_port));
            calcul(2);
            args->m->typeMessage = msgRecu.typeMessage;
            args->m->demandeur.sin_addr = msgRecu.demandeur.sin_addr;
            args->m->demandeur.sin_port = msgRecu.demandeur.sin_port;
            char message[100];
            snprintf(message, 100, "%d:%s:%d:", msgRecu.typeMessage, inet_ntoa(msgRecu.demandeur.sin_addr), msgRecu.demandeur.sin_port);

            recepDemande(args);
        }
        else{ //Si il y a une erreur de frappe
            printf("\nSite %d : Le message reçu n'est ni un jeton ni une demande\n", args->k->num);
            close((*args).socket);
            exit(1);
            etatSite(args->k);
        }
        close(dsExp); //Pas sur
    }
    printf("Je suis sorti du while de reception\n");
    
    pthread_exit(NULL);

}


        
/* Comportement d'un site lors de la réception d'une demande de SC venant du site k */
        
void recepDemande(void * params){

    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;
    
    printf("\n  FONCTION RECEP DEMANDE \n");
    if (args->k->Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine <=> Je suis le seul site qui a pour père lui même
        printf("Site %d : Le site demandeur initial devient mon Next\n", args->k->num);
        args->k->Next.sin_addr.s_addr = args->m->demandeur.sin_addr.s_addr;
        args->k->Next.sin_port = args->m->demandeur.sin_port;
        if (args->k->jeton_present == 1){ //Si j'ai le jeton
            if(args->k->est_demandeur == 0){ //Si je ne suis pas en SC
                printf("Site %d : Mon père devient le site %s:%d\n", args->k->num, inet_ntoa(args->m->demandeur.sin_addr), ntohs(args->m->demandeur.sin_port));
                args->k->Pere.sin_addr.s_addr = args->m->demandeur.sin_addr.s_addr;
                args->k->Pere.sin_port = args->m->demandeur.sin_port;
                args->m->typeMessage = 1;
                args->m->demandeur = args->k->addr;
                printf("Site %d : J'ai le jeton donc je l'envoie à mon Next, le site : %s:%d\n", args->k->num, inet_ntoa(args->k->Next.sin_addr), ntohs(args->k->Next.sin_port));
                envoyerToken(args);
                etatSite(args->k);
            } else if(args->k->est_demandeur == 1){ //Si je suis tjr en SC
                printf("Site %d : J'ai recu une demande et je suis la racine mais je suis en SC, donc il devient mon Next\n", args->k->num);
                printf("Site %d : Mon père devient le site %s:%d", args->k->num, inet_ntoa(args->m->demandeur.sin_addr), ntohs(args->m->demandeur.sin_port));
                args->k->Pere.sin_addr.s_addr = args->m->demandeur.sin_addr.s_addr;
                args->k->Pere.sin_port = args->m->demandeur.sin_port;
                etatSite(args->k);
            }
        } else { //Si j'ai pas le jeton
            printf("Site %d : Il devient mon Next, mais je n'ai pas encore reçu le jeton donc j'attends\n", args->k->num);
            printf("Site %d : Mon père devient le site %s:%d", args->k->num, inet_ntoa(args->m->demandeur.sin_addr), ntohs(args->m->demandeur.sin_port));
            args->k->Pere.sin_addr.s_addr = args->m->demandeur.sin_addr.s_addr;
            args->k->Pere.sin_port = args->m->demandeur.sin_port;          
            etatSite(args->k);
        }
    } else { //Si je ne suis pas la racine
        printf("Site %d : Je n'ai pas le jeton donc j'envoie la demande à mon père, le site : %s:%d\n", args->k->num, inet_ntoa(args->k->addr.sin_addr), ntohs(args->k->addr.sin_port));
        pthread_t threadDemande;
        if (pthread_create(&threadDemande, NULL, envoyerDemande, args) != 0){ //Pas sur si je dois le mettre la ou pas
            perror("Erreur création thread");
            exit(1);
        }      
        pthread_join(threadDemande, NULL);
        //envoyerDemande(args);
        printf("Site %d : Mon père devient le site %s:%d\n", args->k->num, inet_ntoa(args->m->demandeur.sin_addr), ntohs(args->m->demandeur.sin_port));
        args->k->Pere.sin_addr.s_addr = args->m->demandeur.sin_addr.s_addr;
        args->k->Pere.sin_port = args->m->demandeur.sin_port;
        etatSite(args->k);
    }
}
