#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<string.h>


// ce programme est à compléter en fonction des instructions à
// recevoir depuis un processus serveur TCP distant. L'IP et le numéro de
// port du serveur sont fournis en début d'épreuve par les enseignants
// et sont à passer en paramètres de votre programme.

// Une instruction est sous forme de chaine de caractères (caractère
// de fin de chaine inclus) dont la taille (de type int) est à
// recevoir juste avant le texte de l'instruction. Donc, une
// instruction implique deux messages : un premier message qui est la
// taille (en nombre d'octets) du texte de l'instruction et un second
// message qui est le texte de l'instruction.

// Après execution d'une instruction, votre programme recoit un
// feedback avec l'instruction suivante. Là aussi, deux messages sont
// à recevoir : le premier qui est la taille (de type int) en nombre
// d'octets du second mesage qui est une chaine de caractères
// (caractère de fin inclus).

// Remarque 1 : un message de type chaine de caractères ne depassera jamais 6000 octets.


int main(int argc, char *argv[]) {

  	if (argc != 4){
   		//printf("Utilisation : %s ip_serveur port_serveur param3 \n param3 est un entier dont la signification sera fournie par une instruction. En attendant cette instruction, passer n'importe quelle valeur\n", argv[0]);
    	printf("Utilisation : %s ip_serveur port_serveur num_port \n", argv[0]);
 		exit(1);
  	}

  	/* Etape 1 : créer une socket */   
  	int dsClient = socket(PF_INET,SOCK_STREAM,0);

  	if (dsClient == -1){
   		 printf("Client : pb creation socket\n");
    	exit(1); 	// je choisis ici d'arrêter le programme car le reste
	     			// dépendent de la réussite de la création de la socket.
  	}
  
  	printf("Client: creation de la socket : ok\n");

  
  	/* Etape 2 : designer la socket du serveur : avoir une structure qui
     contient l'adresse de cette socket (IP + numéro de port). */
     
  	struct sockaddr_in adrServ;
  	adrServ.sin_addr.s_addr = inet_addr(argv[1]);  //INADDR_ANY;
  	adrServ.sin_family = AF_INET;
 	 adrServ.sin_port = htons(atoi(argv[2])); //htons((short)1110);
    
 	 int lgAdr = sizeof(struct sockaddr_in);
  
    
  	/* Etape 3 : envoyer une demande de connexion au serveur.*/
  
 	 int conn = inet_pton(AF_INET,argv[1], &(adrServ.sin_addr.s_addr));
 	 conn = connect(dsClient, (struct sockaddr *) &adrServ, lgAdr);
  	// je traite les valeurs de retour
  	if (conn <0){
  		perror ("Client: pb au connect :");
 	   	close (dsClient); // je libère les ressources avant de terminer.
   		exit (1); 		// je choisi de quitter le programme : la suite dépend
	      				// de la réussite de l'envoi de la demande de connexion.
  	}

  	printf("Client : demande de connexion reussie \n");


  	// première étape : se connecter au serveur et recevoir la première
  	// instruction (lire commentaire plus haut). Bien évidement, il est
 	// necessaire d'afficher le message reçu pour prendre connaissance
 	// des instructions à suivre pour compléter votre programme.


  	// Recevoir les instructions du serveur

  	// D'abord la taille de l'instruction
  	int tailleInst1;
  	int recevoir = recv (dsClient, &tailleInst1, sizeof(tailleInst1),0);
  
 	if(recevoir < 0){
  		perror("Client: pb de réception:");
  		close(dsClient);
   		exit(1);
 	 }

 	printf("Client : J'ai bien reçu la taille de l'instruction : %d \n",tailleInst1);

 	// Puis l'instruction elle même
 	char inst1[6002];

 	recevoir = recv (dsClient, &inst1, tailleInst1,0);
  
 	if(recevoir < 0){
  		perror("Client: pb de réception:");
  		close(dsClient);
   		exit(1);
 	 }

 	printf("Client : L'instruction du serveur est la suivante : %s \n",inst1);

 	//Je dois renvoyer ce message au serveur

	//Puis j'envoie l'instruction elle même

 	int env = send(dsClient,&inst1,tailleInst1,0);
    if (env < 1) {
        printf("Client : pb à l'envoi de l'instruction\n");
        close (dsClient);
        exit (1);
    }
    
    printf("Client : Instruction transmise \n");


    //Recevoir la deuxième instruction
    // D'abords la taille
    int tailleInst2;
    recevoir = recv (dsClient, &tailleInst2, sizeof(int),0);
  
 	if(recevoir < 0){
  		perror("Client: pb de réception:");
  		close(dsClient);
   		exit(1);
 	 }

 	printf("Client : J'ai bien reçu la taille de l'instruction : %d \n",tailleInst2);


    // Puis le contenu de l'instruction
    char inst2[6002];

    recevoir = recv (dsClient, &inst2, tailleInst2,0);
  
 	if(recevoir < 0){
  		perror("Client : pb de réception :");
  		close(dsClient);
   		exit(1);
 	 }

 	printf("Client : L'instruction du serveur est la suivante : %s \n",inst2);








 	//Socket d'écoute TCP;

 	int dsServeur = socket(PF_INET,SOCK_STREAM,0);

    if (dsServeur == -1){
        perror("Serveur : probleme creation socket");
        exit(1);    // je choisis ici d'arrêter le programme car le reste
                     // dépend de la réussite de la création de la socket.
    }

    
    printf("Serveur: creation de la socket : ok\n");

 	struct sockaddr_in adCli; // pour obtenir l'adresse du client accepté.
 	adCli.sin_family = AF_INET;
    adCli.sin_addr.s_addr = INADDR_ANY;
 	adCli.sin_port = htons(atoi(argv[3]));
    

    if(bind(dsServeur, (struct sockaddr*)&adCli, sizeof(adCli)) < 0){
        perror("Serveur : erreur bind");
        close(dsServeur); // je libère les ressources avant de terminer.
        exit(1); // je choisis de quitter le programme : la suite dépend de la réussite du nommage.
    }
    
    printf("Serveur: nommage : ok\n");

    int ecoute = listen(dsServeur,5);
    if (ecoute < 0){
        printf("Serveur : je suis sourd(e)\n");
        close (dsServeur);
        exit (1);
    }
  
    printf("Serveur: mise en écoute : ok\n");

    int num_p = ntohs(adCli.sin_port);
    //printf("%d\n", num_p);
    env = send(dsClient,&num_p,sizeof(num_p),0); ////////////
    if (env < 1) {
        printf("Client : pb à l'envoi de l'instruction\n");
        close (dsClient);
        exit (1);
    }
    close(dsClient);
    printf("Client : Instruction transmise \n");

    socklen_t lgCli = sizeof (struct sockaddr_in);
    int dsCli = accept(dsServeur, (struct sockaddr *)&adCli, &lgCli);
    if (dsCli < 0){ // je pense toujours à traiter les valeurs de retour.
        perror ( "Serveur : probleme accept :");
        close(dsServeur);
        exit (1);
    }

    printf("Serveur: le client %s:%d est connecté  \n", inet_ntoa(adCli.sin_addr), ntohs(adCli.sin_port));



    // Je recois la taille de l'instruction
    int tailleInst3;
  	int recevoirTCP = recv (dsCli, &tailleInst3, sizeof(tailleInst3),0);
  
 	if(recevoirTCP < 0){
  		perror("Client: pb de réception:");
  		close(dsServeur);
   		exit(1);
 	 }

 	printf("Serveur : J'ai bien reçu la taille de l'instruction : %d \n",tailleInst1);

    // Puis l'instruction elle même
    char inst3[6002];

    recevoirTCP = recv (dsCli, &inst3, tailleInst3,0);
  
 	if(recevoirTCP < 0){
  		perror("Client : pb de réception :");
  		close(dsServeur);
   		exit(1);
 	 }

 	printf("Client : L'instruction du client est la suivante : %s \n",inst3);

 	//Recevoir un message de type chaine de caractere
 	// Lecture d'un entier représentant le nombre d'octets à lire
    int tailleMessage = 0;
    char message[6002];
    int rcv = recv(dsCli,&tailleMessage,sizeof(int),0);
    if (rcv < 0){
        perror("Serveur: probleme reception ");
        close(dsServeur);
        exit(1);
    }
    printf("%s : le client %s:%d doit envoyer %d octets \n", argv[0],inet_ntoa(adCli.sin_addr), ntohs(adCli.sin_port),tailleMessage);
    
    //int nbAlire = sizeof(int);

	rcv = recv (dsCli,&message,tailleMessage,0);
        if (rcv < 0){
            perror("Serveur: probleme reception ");
            close(dsCli);
            exit(1);
        }

    printf("Serveur : contenu du message : %s \n", message);



    


 
  
  
  


  
  
 
}


