all: gerador parque

gerador: gerador.o
	gcc -Wall gerador.o -lpthread -o bin/gerador
gerador.o: gerador.c
	gcc -c gerador.c

parque: parque.o
	gcc -Wall parque.o -lpthread -o bin/parque
parque.o: parque.c
	gcc -c parque.c
