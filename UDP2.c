//
//  UDP2.c
//  Projet_Progra
//
//  Created by Marie-Lou Desbos on 04/12/2021.
//


//Client

/*
 Envoie un message au serveur
 Reçoi un message du serveur
 Envoie n messages
 
 */

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
     
    // je passe en paramètre un numero de port pour la socket du serveur
  
    if (argc != 5){
        printf("utilisation : %s num_processus ip_serveur port_serveur N_nombre_msg_a_envoyer\n", argv[0]);
        exit(0);
    }
    
    int i = atoi(argv[1]); //Numero du processus
  
    /* Créer une socket */
 
    int dC = socket(PF_INET, SOCK_DGRAM, 0);
    if(dC == -1){
        perror("Client : problème création socket");
        exit(1);
    }

    printf("Client : creation de la socket : ok\n");

   /* Nommage de la socket du serveur */
    
    struct sockaddr_in addr_Serv;
    addr_Serv.sin_family=AF_INET;
    inet_pton(AF_INET, argv[2], &(addr_Serv.sin_addr));
    addr_Serv.sin_port = htons((short) atoi(argv[3]));
    socklen_t lgA = sizeof(struct sockaddr_in);
    
    /* Envoi du premier message */
 
    printf("Processus %d : J'envoi un premier message \n", i);
    
    char message[100];
    sprintf(message, "Je suis le premier message");
    printf("%s \n", message);
    
    int snd1 = sendto(dC, &message, sizeof(message), 0, (struct sockaddr*)&addr_Serv, lgA);
    
    if (snd1 <= 0) {
        perror("Client : pb d'envoi ");
        close(dC); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Processus %d : Premier message envoyé \n", i);
    
    /* Reception du message du serveur */
    
    struct sockaddr_in addrExp;
    socklen_t lgAddrExp = sizeof(struct sockaddr_in);
    
    char msgServ[100];
    
    printf("Processus %d: j'attends de recevoir un message du serveur \n", i);
    int rcv = recvfrom(dC, &msgServ, sizeof(msgServ), 0, (struct sockaddr*)&addrExp, &lgAddrExp);
      
    if(rcv < 0){
        perror("Client : Problème au niveau du premier recvfrom");
        close(dC);
        exit(1);
    }
    
    printf("Processus %d : J'ai recu un message : %s \n", i, msgServ);
    
    
    /* Envoie de 3 messages */

    for(int j = 1; j <= atoi(argv[4]); j++){
        
        sprintf(message, "Je suis le message %d sur %d", j, atoi(argv[4]));
    
        int snd = sendto(dC, &message, sizeof(message), 0, (struct sockaddr*)&addr_Serv, lgA);

        /* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
        if (snd <= 0) {
            perror("Client : pb d'envoi : ");
            close(dC); //je libère ressources avant de terminer
            exit(1); //je choisis de quitter le pgm, la suite depend de
            // la reussite de l'envoir de la demande de connexion
        }

        printf("Client : j'ai envoyé le message %d \n", j) ;

    }
    
    /*Terminer proprement. */

    close (dC);
    printf("Processus %d : je termine\n", i);
}
