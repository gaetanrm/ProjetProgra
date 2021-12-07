#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"


sites init(){ //Initialisation de tous les sites au démarrage de l'algo
	//Chacun doit envoyer son num aux autres pour savoir qui est le 1, donc qui sera la racine au départ de l'algo même si il n'a pas fait de demande.
	//C'est juste pour que l'algo puisse fonctionner ensuite car il a besoin d'une racine pour cela
	sites moiMeme;
	moiMeme.Next = NULL; //Faut changer Next et Pere car des int ne marchent pas, il nous faut des structs avzec IP + port
	moiMeme.Pere = 1;
	moiMeme.est_demandeur = 0;
	moiMeme.estEn_SC = 0;
	moiMeme.jeton_present = 0;
	return moiMeme;

}

int envoyerDemande(sites* k){ //Envoie d'une requête de permission pour passer en SC ou passage direct en SC car déjà tête de la liste et pas de queue

	(*k).est_demandeur = 1;

	if ((*k).Pere == NULL){
		//appeler une méthode pour rentrer en SC
	}else{
		/* Créer une socket */
		int ds = socket(PF_INET, SOCK_DGRAM, 0);
		if(ds == -1){
			perror("Serveur : problème création socket");
			exit(1);
		}

		printf("Serveur: creation de la socket : ok\n");

		struct sockaddr_in adresse;
		adresse.sin_family=AF_INET;
		adresse.sin_addr.s_addr = INADDR_ANY;
		adresse.sin_port = htons((*k).Pere);

		if(bind(ds, (struct sockaddr *)&adresse, sizeof(adresse)) < 0){
			perror("Serveur : problème au bind");
			close(ds);
			exit(1);
		}

		printf("Serveur: nommage : ok\n");

		/*Envoie de la demande au père */

		long int message;
		unsigned int nbTotalOctetsEnvoyes = 0;
		unsigned int nbAppelSend = 0;
		int snd = sendto(ds, &message, sizeof(message), 0, (struct sockaddr*)&adresse, sizeof(adresse)); //verif les deux derniers arguments car pas sur
		/* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
		if (snd <= 0) {
			perror("Client:pb d'envoi : ");
			close(ds); //je libère ressources avant de terminer
			exit(1); //je choisis de quitter le pgm, la suite depend de 
			// la reussite de l'envoir de la demande de connexion
		}
		(*k).Pere = NULL;
	}
}

int envoyerToken(sites *k){ //Envoie du token au Next une fois que j'ai fini ce que je voulais faire en SC
	//Gérer socket et envoi avec valeur de retour
}

void finSC(sites* k){ //Sorti de la SC
	(*k).est_demandeur = 0;
	if ((*k).Next != NULL){
		envoyerToken(&k);
		(*k).jeton_present = 0;
		(*k).Next = NULL;
	}
}

void calculSC(){ //Calcul pour simuler une entrée en SC pour un site ayant le token
	//on verra apres pas important pour le moment
}

void recepReq(sites *envoyeur, sites *receveur){ //Comportement d'un site lors de la réception d'une requête venant du site k
	if ((*receveur).Pere == NULL){
		if ((*receveur).est_demandeur == 1){
			(*receveur).Next = (*envoyeur).name;
		}else{
			(*receveur).jeton_present = 0;
			envoyerToken(&envoyeur);
		}
	}else{
		envoyerDemande(&envoyeur);
	}
	(*receveur).Pere = (*envoyeur).name;
}

void recepToken(sites* k){ //Comportement lors de la réception du token par un site l'ayant demandé
	(*k).jeton_present = 1;
}

int main(int argc, char *argv[]){


	// je passe en paramètre un numero de port pour la socket du serveur
  
  	if (argc<2){
    	printf("utilisation: %s numero_port\n", argv[0]);
    	exit(1);
  	}

  	/* Créer une socket */
 
  	int ds = socket(PF_INET, SOCK_DGRAM, 0);
  	if(ds == -1){
  		perror("Serveur : problème création socket");
  		exit(1);
  	}

 	 printf("Serveur: creation de la socket : ok\n");

  	struct sockaddr_in adresse;
  	adresse.sin_family=AF_INET;
  	adresse.sin_addr.s_addr = INADDR_ANY;
  	adresse.sin_port = htons((short) atoi(argv[1]));

 	if(bind(ds, (struct sockaddr *)&adresse, sizeof(adresse)) < 0){
  		perror("Serveur : problème au bind");
  		close(ds);
  		exit(1);
  	}

  	printf("Serveur: nommage : ok\n");

}