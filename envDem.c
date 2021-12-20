//
//  envDem.c
//  
//
//  Created by Marie-Lou Desbos on 20/12/2021.
//

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
#include "envDem.h"

int envoyerDemande(sites* sommet, message* msg, int socket){     //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
                                //resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre

    if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
        printf("Site %d : Je suis dans la fonction envoyerDemande : je deviens demandeur\n", (*sommet).num);
        (*sommet).est_demandeur = 1;
    }

    if ((*sommet).Pere.sin_addr.s_addr == inet_addr("0.0.0.0")){ //Si je suis la racine
        return 1;
        
    } else { //Envoie la demande à son père
        printf("Site %d : J'envoi la demande à mon père, le processus %s:%d\n", (*sommet).num, inet_ntoa((*sommet).Pere.sin_addr), ntohs((*sommet).Pere.sin_port));
        sockaddr_in addrPere = (*sommet).Pere;
        socklen_t lgAddrPere = sizeof(sockaddr_in);
        
        ssize_t snd = sendto(socket, msg, sizeof(struct message), 0, (struct sockaddr*)&addrPere, lgAddrPere);
        /* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
        if (snd <= 0) {
            perror("Client:pb d'envoi : ");
            close(socket);
            exit(1);
        }

        if (((*sommet).addr.sin_addr.s_addr == (*msg).demandeur.sin_addr.s_addr) && (sommet->addr.sin_port == msg->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
            printf("Site %d : Je met mon père à null car j'ai envoyé une demande donc je me concidère comme la racine\n", (*sommet).num);
            (*sommet).Pere.sin_addr.s_addr = inet_addr("0.0.0.0");
            (*sommet).Pere.sin_port = 0;
        }
        
        return 0;
    }
}
