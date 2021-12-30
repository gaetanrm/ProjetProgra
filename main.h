#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

typedef struct sites sites;
typedef struct sockaddr_in sockaddr_in;
typedef struct in_addr in_addr;
typedef struct message message;

struct sites {
	sockaddr_in addr;
	sockaddr_in Next;
	sockaddr_in Pere; //Site suivant dans la liste d'attente et site père du site
	int jeton_present, est_demandeur, num; //Booléen pour savoir si le token est présent et si le site est demandeur.
};

struct message {
    int typeMessage; // 0 si c'est une demande, 1 si c'est le jeton
    sockaddr_in demandeur;
};

// structure pour regrouper les paramètres d'un thread. 
struct paramsFonctionThread {
	int socket;
    message *m;
    sites *k;   
    pthread_mutex_t lock;
    pthread_cond_t a_jeton;
};

void init(sites *sommet, int port, in_addr IP_Pere, int Port_p, int num, int rac);//Initialisation de tous les sites au démarrage de l'algo

void * envoyerDemande(void * params); //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue

void envoyerToken(void * params); //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC

void finSC(sites* k, int socket); //Sorti de la SC

void recepDemande(void * params);//Comportement d'un site lors de la réception d'une requête venant du site k

void * reception(void* params); //Réception d'un message et choix entre token et demande

void envoyer(message msg);

void etatSite(sites *s);


/*
Structures de donnée choisi :
	Une liste pour la liste d'attente
	Une structure pour chaque site contenant les Next et Pere ainsi que les booléens qui lui sont propres. Ainsi, il est plus facile de manipuler les sites.
	int Next et Pere qui contiennent les noms des noeuds du père et du suivant
	Un booléen pour le jeton, un booléen pour la SC, un booléen pour savoir si le site est en demande de la SC ou non
	Des envois de message via le réseau et les sockets pour les requêtes de demande de permission
	Des envois de message pour transmettre le jeton  
*/

#endif
