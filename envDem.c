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
#include "calcul.h"

void * envoyerDemande(void * params){     //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
                                //resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre
    
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;
    
    printf("\n  FONCTION ENVOYER DEMANDE \n");
    
    if (((*args->k).addr.sin_addr.s_addr == (*args->m).demandeur.sin_addr.s_addr) && ((*args->k).addr.sin_port == (*args->m).demandeur.sin_port)){ //Si je suis le demandeur 'initial'
        printf("Site %d : Je suis dans la fonction envoyerDemande : je deviens demandeur\n", (*args->k).num);
        (*args->k).est_demandeur = 1;
    }
    
    //Si mon père est différent de null
    
    if ((*args->k).Pere.sin_addr.s_addr != inet_addr("0.0.0.0")) {
        //Envoie la demande à son père
        printf("Site %d : J'envoi la demande à mon père, le processus %s:%d\n", (*args->k).num, inet_ntoa((*args->k).Pere.sin_addr), ntohs((*args->k).Pere.sin_port));

        
        /* TCP */
        
        //Création de la socket d'envoi
        int dSPere = socket(PF_INET, SOCK_STREAM, 0);

        if(dSPere == -1){
            perror("Problème création socket du père dans envoyerDemande ");
            exit(1);
        }
        
        printf("Site %d : création de la socket pour communiquer avec mon père ok \n", (*args->k).num);
        
        sockaddr_in addrPere;
        addrPere.sin_addr = (*args->k).Pere.sin_addr;
        addrPere.sin_port = (*args->k).Pere.sin_port;
        addrPere.sin_family = AF_INET;
        socklen_t lgAddrPere = sizeof(struct sockaddr_in);

        
        //envoyer une demande de connexion au père.
        int conn = connect(dSPere, (struct sockaddr *) &addrPere, lgAddrPere);
        
        if (conn < 0){
            perror("pb au connect dans la fonction envoyerDemande ");
            close((*args->socket));
            exit(1);
        }

        printf("Site %d : Demande de connexion à mon père reussie, je suis connecté au site %s:%d \n", (*args->k).num, inet_ntoa(addrPere.sin_addr), ntohs(addrPere.sin_port));
        
        
        char message[100];
        snprintf(message, 100, "%d:%s:%d:", (*args->m).typeMessage, inet_ntoa((*args->m).demandeur.sin_addr), (*args->m).demandeur.sin_port);
        printf("Type du message que j'envoie : %d\n", (*args->m).typeMessage);
        
        //Puis j'envoie l'instruction elle même
        ssize_t env = send(dSPere, &message, sizeof(struct message),0);
        if (env < 1) {
            printf("Site %d : pb à l'envoi de la demande\n", (*args->k).num);
            close (dSPere);
            exit (1);
        }
        
        printf("Site %d : Demande transmise à mon père \n", (*args->k).num);
        
        close(dSPere); //Pas sur
        
        /* FIN TCP */
        
        /* UDP
         
        ssize_t snd = sendto(s, msg, sizeof(struct message), 0, (struct sockaddr*)&addrPere, lgAddrPere);
        
        if (snd <= 0) {
            perror("Client:pb d'envoi : ");
            close(s);
            exit(1);
        }
        
        FIN UDP */

            
        if (((*args->k).addr.sin_addr.s_addr == (*args->m).demandeur.sin_addr.s_addr) && ((*args->k).addr.sin_port == (*args->m).demandeur.sin_port)){ //Si je suis le demandeur 'initial'
            // Je met mon père à null
            printf("Site %d : Je met mon père à null car j'ai envoyé une demande donc je me concidère comme la racine\n", (*args->k).num);
            (*args->k).Pere.sin_addr.s_addr = inet_addr("0.0.0.0");
            (*args->k).Pere.sin_port = 0;
            
            // J'attends d'avoir le jeton
            printf("Site %d : J'attends d'avoir le jeton pour entrer en SC\n", (*args->k).num);
            pthread_mutex_lock(&args->jeton);
            pthread_cond_wait(&args->a_jeton, &args->jeton);
            printf("\n ~~~~~ SECTION CRITIQUE ~~~~~\n");
            
            printf("\nSite %d : J'ai le jeton donc je rentre en section critique\n", (*args->k).num);
            calcul(7);
            
            printf("Pour sortir de la SC tapez 1 : ");
            int fSC = 0;
            scanf("%d", &fSC);
            int i = 0;
            while (fSC != 1) {
                i++;
            }
            
            printf("Site %d : J'ai terminé ma Section Critique\n", (*args->k).num);
            
            finSC(args->k, (*args->socket));
            
            printf("\n ~~~~~ FIN SECTION CRITIQUE ~~~~~\n");
            
            
                
            //J'ai le jeton donc je rentre en SC
                
        }
        
    } else if ((*args->k).Pere.sin_addr.s_addr == inet_addr("0.0.0.0") && (*args->k).jeton_present == 1) { //Si mon père est null
        printf("\n ~~~~~ SECTION CRITIQUE ~~~~~\n");
        
        printf("\nSite %d : J'ai déjà le jeton donc je rentre en section critique\n", (*args->k).num);
        calcul(7);
        
        printf("Pour sortir de la SC tapez 1 : ");
        int fSC = 0;
        scanf("%d", &fSC);
        int i = 0;
        while (fSC != 1) {
            i++;
        }
        
        printf("Site %d : J'ai terminé ma Section Critique\n", (*args->k).num);
        
        finSC(args->k, (*args->socket));
        
        printf("\n ~~~~~ FIN SECTION CRITIQUE ~~~~~\n");
        
    }
        pthread_exit(NULL);
        
}
        

