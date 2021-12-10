#include <stdio.h>

typedef struct sites sites;
typedef struct parent parent;
typedef struct in_addr in_addr;

struct parent{
	char* IP;
	int port;
};
struct sites {
	char* IP;
	int port;
	parent Next;
	parent Pere; //Site suivant dans la liste d'attente et site père du site
	int jeton_present, est_demandeur, estEn_SC; //Booléen pour savoir si le token est présent, si le site est demandeur et si le site est en SC.
};

sites init(int port);//Initialisation de tous les sites au démarrage de l'algo

int envoyerDemande(sites *k); //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue

int envoyerToken(sites *k); //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC

void finSC(sites* k); //Sorti de la SC

void calculSC();//Calcul pour simuler une entrée en SC pour un site ayant le token

void recepDemande(parent *envoyeur, sites *receveur);//Comportement d'un site lors de la réception d'une requête venant du site k

void reception(sites *k); //Réception d'un message et choix entre token et demande

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