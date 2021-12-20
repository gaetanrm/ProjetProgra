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
#include "calcul.h"

void calcul(int a){

    for(int i=0; i<a; i++){
        int sum;
        for (int i = 0; i<10000; i++) {
            sum = i*4000;
        }
        printf("...\n");
    }
}
