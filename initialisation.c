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

/* Initialisation de tous les sites au démarrage de l'algo */

void init(sites *sommet, int port, in_addr IP_p, int Port_p, int n, int rac){
    
    
    //sites sommet;
    (*sommet).num = n;
    
    printf("\n~~~~~ INITIALISATION DU SITE %d ~~~~~\n\n", (*sommet).num);
    
    
    //addr
    (*sommet).addr.sin_family = AF_INET;
    (*sommet).addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //TODO à ne pas mettre en dur
    (*sommet).addr.sin_port = htons((short)port);
    printf("IP du sommet: %s\n", inet_ntoa((*sommet).addr.sin_addr));
    printf("Port du sommet: %hu\n", ntohs((*sommet).addr.sin_port));
    
    //Next
    (*sommet).Next.sin_family = AF_INET;
    (*sommet).Next.sin_addr.s_addr = inet_addr("0.0.0.0"); //Equivalent à NULL
    (*sommet).Next.sin_port = 0;
    printf("IP du Next: %s\n", inet_ntoa((*sommet).Next.sin_addr));
    printf("Port du Next: %hu\n", ntohs((*sommet).Next.sin_port));
    
    //Pere
    (*sommet).Pere.sin_family = AF_INET;
    
    if ((*sommet).num != rac) { //Si je ne suis pas la racine
        (*sommet).Pere.sin_addr = IP_p;
        printf("IP du Père: %s\n", inet_ntoa((*sommet).Pere.sin_addr));
        (*sommet).Pere.sin_port = htons((short)Port_p);
        printf("Port du Père: %hu\n", ntohs((*sommet).Pere.sin_port));
        (*sommet).jeton_present = 0;
    } else { // SI je suis la racine mon père est null
        (*sommet).Pere.sin_addr.s_addr = inet_addr("0.0.0.0"); //Valeur aléatoire, faudra s'en occuper plus tard
        (*sommet).Pere.sin_port = 0;
        printf("IP du père: %s\n", inet_ntoa((*sommet).Pere.sin_addr));
        printf("Port du Père: %hu\n", ntohs((*sommet).Pere.sin_port));
        (*sommet).jeton_present = 1;
    }
    
    (*sommet).est_demandeur = 0;
    printf("\nEst demandeur ? : non\n");
    
    if((*sommet).num == rac){ //Si je suis la racine j'ai le jeton
        (*sommet).jeton_present = 1;
        printf("Jeton présent ? : Oui\n");
    } else {
        printf("Jeton présent ? : non\n");
    }

    
    printf("\n~~~~~ SITE %d INITIALISÉ ~~~~~\n", (*sommet).num);
    
}
