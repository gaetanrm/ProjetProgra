#ifndef recep_h
#define recep_h

#include <stdio.h>
#include "main.h"

void * reception(void * params);
void recepDemande(message* msg, sites *k, int socket);

#endif /* recep_h */
