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

void envoyerToken(sites *k, message* msg, int socket){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
    //Gérer socket et envoi avec valeur de retour

    printf("Processus %d : J'envoi le jeton\n", (*k).num);
    
    int snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&k->Next, sizeof(struct sockaddr_in));    //SI pas de next, erreur a l'éxécution !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
    if (snd <= 0) {
        perror("pb d'envoi du jeton\n");
        close(socket); //je libère ressources avant de terminer
        exit(1); //je choisis de quitter le pgm, la suite depend de
        // la reussite de l'envoir de la demande de connexion
    }
    
    printf("Site %d : Jeton envoyé au processus %s:%d\n", (*k).num, inet_ntoa((*k).Next.sin_addr), ntohs((*k).Next.sin_port));
    
    //J'ai envoyé le jeton à mon Next donc je met mon Next à null
    (*k).Next.sin_addr.s_addr = inet_addr("0.0.0.0");
    (*k).Next.sin_port = 0;
    
    (*k).jeton_present = 0;
}
