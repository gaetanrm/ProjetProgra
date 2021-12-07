//
//  UDP.c
//  Projet_Progra
//
//  Created by Marie-Lou Desbos on 04/12/2021.
//


//Envoie un réception d'un message en UDP sur une même machine

// Serveur - ne connais pas l'adresse du processus qui va lui envoyer des messages

/*
 Attend de recevoir un message
 Envoie un message à son tour
 Se met en attente de recevoir d'un nombre indéterminé de messages
 */

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
    
    if (argc != 3){
        printf("utilisation : %s num_processus num_port \n", argv[0]);
        exit(0);
    }
    
    int i = atoi(argv[1]); //Numero du processus
    
    /* Créer une socket */
   
    int ds = socket(PF_INET, SOCK_DGRAM, 0);
    if(ds == -1){
        perror("Serveur : problème création socket");
        exit(1);
    }

    printf("Processus %d : creation de la socket : ok\n", i);
    
    /* Nommage de la socket */
    
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons((short) atoi(argv[2]));
    
    //bind : associer une adresse IP et un num de port à la socket
    int b = bind(ds, (struct sockaddr *)&ad, sizeof(ad));
    
    if(b < 0){
        perror("Processus : problème au bind");
        close(ds);
        exit(1);
    }

    printf("Processus %i : nommage : ok\n", i);
    
    /* Réception des messages : chaque message est un long int */
    
    //socklen_t lgAddrAd = sizeof(struct sockaddr_in);
   
    char msg[100];
    
    // variables utilisées pour récupérer l'adresse de l'expéditeur.
    struct sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);
    
    
    printf("Processus %d : j'attends de recevoir un premier message\n", i);
    
    int rcv = recvfrom(ds, &msg, sizeof(msg), 0, (struct sockaddr*)&addrExp, &lgAddrExp);
      
    if(rcv < 0){
        perror("Serveur : Problème au niveau du premier recvfrom");
        close(ds);
        exit(1);
    }

    // qui a envoyé le message reçu ?
    //char *IP_Exp = inet_ntoa(addrExp.sin_addr);
    //char Port_Exp = ntohs(addrExp.sin_port);
    
    printf("Processus %d : le processus %s:%d m'a envoyé un message  \n", i, inet_ntoa(addrExp.sin_addr), ntohs(addrExp.sin_port));
    
    
    /* Envoyer un message au client pour lui dire qu'on a bien reçu son message */
    
    char message[100];
    sprintf(message, "Je suis le serveur et j'ai bien reçu ton message");
    
    int snd = sendto(ds, &message, sizeof(message), 0, (struct sockaddr*)&addrExp, lgAddrExp);
    
    if (snd <= 0) {
        perror("Client : pb d'envoi : ");
        close(ds); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }

    printf("Processus %d : j'ai envoyé le message au client \n", i) ;
    
    /* Attendre des messages indéfiniment */
    
    while(1){ // le serveur n'a pas connaissance du nombre de messages qu'il recevra, d'où la boucle infinie.
        
        char msgRecu[100];
        
        rcv = recvfrom(ds, &msgRecu, sizeof(msgRecu), 0,  (struct sockaddr*)&addrExp, &lgAddrExp);

        if(rcv < 0){
            perror("Processus : Problème dans un recvfrom de la boucle while");
        }

        // qui a envoyé le message reçu ?
        printf("Processus %d : le processus %s:%d m'a envoyé un message: %s \n", i, inet_ntoa(addrExp.sin_addr),  ntohs(addrExp.sin_port), msgRecu);
    
    }
    
    /* je termine proprement */
    close(ds);
    printf("Processus %d : je termine\n", i);
}
