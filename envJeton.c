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

/* Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC */

void envoyerToken(void * params){

    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;

    printf("\n FONCTION ENVOYER JETON \n");
    
    
    /* TCP */
    
    int dsNext = socket(PF_INET, SOCK_STREAM, 0);

    if(dsNext == -1){
        perror("Problème création socket du Next dans envoyerToken");
        exit(1);
    }
    
    //envoyer une demande de connexion au Next.
    sockaddr_in addrNext = args->k->Next;
    socklen_t lgAddrNext = sizeof(sockaddr_in);
    
    int conn = addrNext.sin_addr.s_addr;
    conn = connect(dsNext, (struct sockaddr *) &addrNext, lgAddrNext);
    
    if (conn < 0){
        perror ("pb au connect dans la fonction envoyerJeton ");
        close(args->socket);
        exit (1);
    }

    printf("Site %d : demande de connexion à mon Next reussie \n", args->k->num);
    
    printf("Mon next est le site : %s:%d\n", inet_ntoa(args->k->Next.sin_addr), ntohs(args->k->Next.sin_port));
    
    
    char msgJ[100];
    snprintf(msgJ, 100, "%d:%s:%d:", args->m->typeMessage, inet_ntoa(args->m->demandeur.sin_addr), args->m->demandeur.sin_port);
    
    //Puis j'envoie le jeton
    int env = send(dsNext, &msgJ, sizeof(msgJ),0);
    if (env < 1) {
        printf("Site %d : pb à l'envoi de la demande\n", args->k->num);
        close (dsNext);
        exit (1);
    }
    
    printf("Site %d : Jeton envoyé au site %s:%d\n", args->k->num, inet_ntoa(args->k->Next.sin_addr), ntohs(args->k->Next.sin_port));

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
    args->k->Next.sin_addr.s_addr = inet_addr("0.0.0.0");
    args->k->Next.sin_port = 0;
    
    args->k->jeton_present = 0;
    close(dsNext);
    
    printf("\n FIN FONCTION ENVOYER JETON \n");

}
