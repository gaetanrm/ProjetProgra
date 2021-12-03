#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>


void envoyerDemande(int pere, bool *demande){

	*demande = true;

	if (pere == null){
		//rentrer en SC
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
  	adresse.sin_port = htons(pere);

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

  for(int i = 1; i <= atoi(argv[3]); i++){
    message = i; // pour passer d'un int à long int (de 4 à 8 octets). Vous pouvez faire autrement.
    
    int snd = sendto(ds, &message, sizeof(message), 0, (struct sockaddr*)&ad, lgA);

    /* Traiter TOUTES les valeurs de retour (voir le cours ou la documentation). */
    if (snd <= 0) {
      perror("Client:pb d'envoi : ");
      close(ds); //je libère ressources avant de terminer
      exit(1); //je choisis de quitter le pgm, la suite depend de 
       // la reussite de l'envoir de la demande de connexion
    }

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