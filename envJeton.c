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
#include "envJeton.h"

void envoyerToken(sites *k, message* msg, int s){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour

    printf("\n  FONCTION ENVOYER JETON \n");
    
    
    /* TCP */
    
    int dsNext = socket(PF_INET, SOCK_STREAM, 0);

    if(dsNext == -1){
        perror("Problème création socket du Next dans envoyerToken");
        exit(1);
    }
    
    //envoyer une demande de connexion au Next.
    sockaddr_in addrNext = (*k).Next;
    socklen_t lgAddrNext = sizeof(sockaddr_in);
    
    int conn = addrNext.sin_addr.s_addr;
    conn = connect(dsNext, (struct sockaddr *) &addrNext, lgAddrNext);
    
    if (conn < 0){
        perror ("pb au connect dans la fonction envoyerJeton ");
        close(s);
        exit (1);
    }

    printf("Site %d : demande de connexion à mon Next reussie \n", (*k).num);
    
    /*
     
    //J'envoie la taille de l'instruction
    int tailleInst = sizeof(msg); // TODO *msg ou juste msg ??
    
    ssize_t env = send(dsNext, &tailleInst,tailleInst,0);
    if (env < 1) {
        printf("Site %d : pb à l'envoi de la taille de l'instruction dans envoyerDemande\n", (*k).num);
        close (dsNext);
        exit (1);
    }
    
    printf("Site %d : taille du message transmise \n", (*k).num);
     
     */
    
    char msgJ[100];
    snprintf(msgJ, 100, "%d:%s:%d:", msg->typeMessage, inet_ntoa(msg->demandeur.sin_addr), msg->demandeur.sin_port);
    
    //Puis j'envoie le jeton
    int env = send(dsNext, &msgJ, sizeof(msgJ),0);
    if (env < 1) {
        printf("Site %d : pb à l'envoi de la demande\n", (*k).num);
        close (dsNext);
        exit (1);
    }
    
    printf("Site %d : Jeton envoyé au processus %s:%d\n", (*k).num, inet_ntoa((*k).Next.sin_addr), ntohs((*k).Next.sin_port));

    /* FIN TCP */
    
    
    /* UDP
    int snd = sendto(s, msg, sizeof(struct message), 0, (struct sockaddr*)&k->Next, sizeof(struct sockaddr_in));    //SI pas de next, erreur a l'éxécution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton\n");
        close(s);
        exit(1);
        //
    }
    
    FIN UDP */
    
        
    //J'ai envoyé le jeton à mon Next donc je met mon Next à null
    (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
    (*k).Next.sin_port = 0;
    
    (*k).jeton_present = 0;
    close(dsNext);
}
