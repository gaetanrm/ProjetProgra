all: site clean

site: site.o calcul.o envDem.o envJeton.o initialisation.o recep.o
	gcc -o site site.o calcul.o envDem.o envJeton.o initialisation.o recep.o -lpthread

site.o: main.c
	gcc -o site.o -c main.c -Wall

calcul.o: calcul.c calcul.h
	gcc -o calcul.o -c calcul.c -Wall

envDem.o: envDem.c envDem.h
	gcc -o envDem.o -c envDem.c -Wall

envJeton.o: envJeton.c envJeton.h
	gcc -o envJeton.o -c envJeton.c -Wall

initialisation.o: initialisation.c initialisation.h
	gcc -o initialisation.o -c initialisation.c -Wall

recep.o: recep.c recep.h
	gcc -o recep.o -c recep.c -Wall



clean:
	rm -f site.o calcul.o envDem.o envJeton.o initialisation.o recep.o