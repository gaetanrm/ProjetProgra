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

sites init(int port, in_addr IP_p, int Port_p, int num, int rac){ //Initialisation de tous les sites au démarrage de l'algo
    //Chacun doit envoyer son num aux autres pour savoir qui est le 1, donc qui sera la racine au départ de l'algo même si il n'a pas fait de demande.
    //C'est juste pour que l'algo puisse fonctionner ensuite car il a besoin d'une racine pour cela
    
    sites sommet;
    
    sommet.num = num;
    
    //addr
    sommet.addr.sin_family = AF_INET;
    sommet.addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //TODO à ne pas mettre en dur
    sommet.addr.sin_port = htons((short)port);
    printf("IP du sommet: %s\n", inet_ntoa(sommet.addr.sin_addr));
    printf("Port du sommet: %hu\n", ntohs(sommet.addr.sin_port));
    
    //Next
    sommet.Next.sin_family = AF_INET;
    sommet.Next.sin_addr.s_addr = inet_addr("0.0.0.0"); //Equivalent à NULL
    sommet.Next.sin_port = 0;
    printf("IP du Next: %s\n", inet_ntoa(sommet.Next.sin_addr));
    printf("Port du Next: %hu\n", ntohs(sommet.Next.sin_port));
    
    //Pere
    sommet.Pere.sin_family = AF_INET;
    
    if (sommet.num != rac) {
        sommet.Pere.sin_addr = IP_p;
        printf("IP du père: %s\n", inet_ntoa(sommet.Pere.sin_addr));
        sommet.Pere.sin_port = htons((short)Port_p);
        printf("Port du Pèret: %hu\n", ntohs(sommet.Pere.sin_port));
        sommet.jeton_present = 0;
    } else {
        sommet.Pere.sin_addr.s_addr = inet_addr("0.0.0.0"); //Valeur aléatoire, faudra s'en occuper plus tard
        sommet.Pere.sin_port = 0; //Idem
        printf("IP du père: %s\n", inet_ntoa(sommet.Pere.sin_addr));
        printf("Port du Père: %hu\n", ntohs(sommet.Pere.sin_port));
        sommet.jeton_present = 1;
    }
    
    sommet.est_demandeur = 0;
    sommet.estEn_SC = 0;
    
    printf("\nSite %d initialisé\n", sommet.num);
    //printf("\nIP : %s \nPort : %d \nIP du père : %s \nPort du père : %d \nIP du Next : %u \nPort du Next : %d \nJeton présent ? %d\n", inet_ntoa(sommet.addr.sin_addr), ntohs(sommet.addr.sin_port), inet_ntoa(sommet.Pere.sin_addr), ntohs(sommet.Pere.sin_port), sommet.Next.sin_addr.s_addr, sommet.Next.sin_port, sommet.jeton_present);
    
    return sommet;
}
