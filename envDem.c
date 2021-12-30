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

/* Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue
 * Resultat: 1 si il est la racine, 0 si il a envoyé la demande à qq d'autre*/

void * envoyerDemande(void * params){
    
    struct paramsFonctionThread * args = (struct paramsFonctionThread *) params;
    
    printf("\n  FONCTION ENVOYER DEMANDE \n");
    
    if ((args->k->addr.sin_addr.s_addr == args->m->demandeur.sin_addr.s_addr) && (args->k->addr.sin_port == args->m->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
        printf("Site %d : Je deviens demandeur de section critique\n", args->k->num);
        args->k->est_demandeur = 1;
    }
    
    //Si mon père est différent de null <=> Si je ne suis pas la racine
    
    if (args->k->Pere.sin_addr.s_addr != inet_addr("0.0.0.0")) {
        //Envoie la demande à son père
        printf("Site %d : J'envoie la demande à mon père, le site %s:%d\n", args->k->num, inet_ntoa(args->k->Pere.sin_addr), ntohs(args->k->Pere.sin_port));
        
        /* TCP */
        
        //Création de la socket d'envoi
        int dSPere = socket(PF_INET, SOCK_STREAM, 0);

        if(dSPere == -1){
            perror("Problème création socket du père dans envoyerDemande ");
            exit(1);
        }
        
        printf("Site %d : Création de la socket pour communiquer avec mon Père ok \n", args->k->num);
        
        sockaddr_in addrPere;
        addrPere.sin_addr = args->k->Pere.sin_addr;
        addrPere.sin_port = args->k->Pere.sin_port;
        addrPere.sin_family = AF_INET;
        socklen_t lgAddrPere = sizeof(struct sockaddr_in);

        
        //envoyer une demande de connexion au père.
        int conn = connect(dSPere, (struct sockaddr *) &addrPere, lgAddrPere);
        
        if (conn < 0){
            perror("pb au connect dans la fonction envoyerDemande ");
            close(args->socket);
            exit(1);
        }

        printf("Site %d : Demande de connexion à mon père reussie, je suis connecté au site %s:%d \n", args->k->num, inet_ntoa(addrPere.sin_addr), ntohs(addrPere.sin_port));
        
        
        char message[100];
        snprintf(message, 100, "%d:%s:%d:", args->m->typeMessage, inet_ntoa(args->m->demandeur.sin_addr), args->m->demandeur.sin_port);
        
        //Puis j'envoie la demande de section critique
        ssize_t env = send(dSPere, &message, sizeof(struct message),0);
        if (env < 1) {
            printf("Site %d : pb à l'envoi de la demande\n", args->k->num);
            close (dSPere);
            exit (1);
        }
        
        printf("Site %d : Demande transmise à mon père \n", args->k->num);
        
        etatSite(args->k);
        
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

            
        if ((args->k->addr.sin_addr.s_addr == args->m->demandeur.sin_addr.s_addr) && (args->k->addr.sin_port == args->m->demandeur.sin_port)){ //Si je suis le demandeur 'initial'
            // Je met mon père à null
            printf("\nSite %d : Je met mon père à null car j'ai envoyé une demande donc je me concidère comme la racine\n", args->k->num);
            args->k->Pere.sin_addr.s_addr = inet_addr("0.0.0.0");
            args->k->Pere.sin_port = 0;
            
            // J'attends d'avoir le jeton
            printf("Site %d : J'attends d'avoir le jeton pour entrer en section critique\n", args->k->num);
            etatSite(args->k);
            pthread_mutex_lock(&args->lock);
            pthread_cond_wait(&args->a_jeton, &args->lock);
            
            //J'ai le jeton donc je rentre en SC
            printf("\nSite %d : J'ai le jeton donc je rentre en section critique\n", args->k->num);
            
            printf("\n ~~~~~ SECTION CRITIQUE ~~~~~\n");
            
            calcul(7);
            
            etatSite(args->k);
            
            printf("\nPour sortir de la SC tapez 1 : \n");
            int fSC = 0;
            scanf("%d", &fSC);
            int i = 0;
            while (fSC != 1) {
                i++;
            }
            
            printf("\nSite %d : J'ai terminé ma section critique\n", args->k->num);
            
            finSC(args->k, args->socket);
            
            printf("\n ~~~~~ FIN SECTION CRITIQUE ~~~~~\n");
            
            etatSite(args->k);
            
            
                
            
                
        }
        
    } else if (args->k->Pere.sin_addr.s_addr == inet_addr("0.0.0.0") && args->k->jeton_present == 1) { //Si mon père est null
        printf("\nSite %d : J'ai déjà le jeton donc je rentre en section critique\n", args->k->num);
        printf("\n ~~~~~ SECTION CRITIQUE ~~~~~\n");
       
        calcul(7);
        
        etatSite(args->k);
        
        printf("\nPour sortir de la SC tapez 1 : \n");
        int fSC = 0;
        scanf("%d", &fSC);
        int i = 0;
        while (fSC != 1) {
            i++;
        }
        
        printf("Site %d : J'ai terminé ma section critique\n", args->k->num);
        
        finSC(args->k, args->socket);
        
        printf("\n ~~~~~ FIN SECTION CRITIQUE ~~~~~\n");
        
        etatSite(args->k);
    }
    
    pthread_exit(NULL);
        
}
        

