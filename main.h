#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

typedef struct sites sites;
typedef struct sockaddr_in sockaddr_in;
typedef struct in_addr in_addr;

struct sites {
	sockaddr_in addr;
	sockaddr_in Next;
	sockaddr_in Pere; //Site suivant dans la liste d'attente et site père du site
	int jeton_present, est_demandeur, estEn_SC, num; //Booléen pour savoir si le token est présent, si le site est demandeur et si le site est en SC.
};

struct predicatRdv {
  pthread_mutex_t lock;
  pthread_cond_t have_jeton;
  int nbSitesAvecToken;
};

// structure pour regrouper les paramètres d'un thread. 
struct paramsFonctionThread {
	int idThread;
	sites *k;
	int socket;
	struct predicatRdv * varPartagee;
};


sites init(int port, in_addr IP_Pere, int Port_p, int num, int rac);//Initialisation de tous les sites au démarrage de l'algo

int envoyerDemande(sites *k, int socket); //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue

void envoyerToken(sites *k, int socket); //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC

void finSC(sites* k, int socket); //Sorti de la SC

void calculSC();//Calcul pour simuler une entrée en SC pour un site ayant le token

void recepDemande(sites *demandeur, sites *k, int socket);//Comportement d'un site lors de la réception d'une requête venant du site k

void * reception(void* params); //Réception d'un message et choix entre token et demande

//void recepToken(sites* k);//Comportement lors de la réception du token par un site l'ayant demandé

in_addr** connaitreIP();

/*
Structure de donnée choisi :
	Une liste pour la liste d'attente
	Une structure pour chaque site contenant les Next et Pere ainsi que les booléens qui lui sont propres. Ainsi, il est plus facile de manipuler les sites.
	int Next et Pere qui contiennent les noms des noeuds du père et du suivant
	Un booléen pour le jeton, un booléen pour la SC, un booléen pour savoir si le site est en demande de la SC ou non
	Des envois de message via le réseau et les sockets pour les requêtes de demande de permission
	Des envois de message pour transmettre le jeton  
*/

#endif
