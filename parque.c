#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <semaphore.h>
#include <fcntl.h>
#include "structs.h"

#define CONTR_N_ID	0
#define CONTR_S_ID	1
#define CONTR_E_ID	2
#define CONTR_O_ID	3

#define FIFON	"/tmp/fifoN"
#define FIFOS	"/tmp/fifoS"
#define FIFOE	"/tmp/fifoE"
#define FIFOO	"/tmp/fifoO"

void * tarrumador(void * arg){
	//arruma
	
	//sleep(duration);
	
	//manda embora
	
	return NULL;
}

void * tcontrollerN(void * arg){
	int closed = 0;
	Viatura * vehicle = (Viatura *) (arg);
	int fd = open(FIFON, O_RDONLY);
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){
			//enviar para trás "fechado"
		}
		else {
			//criar thread arrumador e passar vehicle
		}

	}
	close(fd);
	return NULL;
}

void * tcontrollerS(void * arg){
	int closed = 0;
	Viatura * vehicle = (Viatura *) (arg);
	int fd = open(FIFOS, O_RDONLY);
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){
			//enviar para trás "fechado"
		}
		else {
			//criar thread arrumador e passar vehicle
		}

	}
	close(fd);
	return NULL;
}

void * tcontrollerE(void * arg){
	int closed = 0;
	Viatura * vehicle = (Viatura *) (arg);
	int fd = open(FIFOE, O_RDONLY);
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){
			//enviar para trás "fechado"
		}
		else {
			//criar thread arrumador e passar vehicle
		}

	}
	close(fd);
	return NULL;
}

void * tcontrollerO(void * arg){
	int closed = 0;
	Viatura * vehicle = (Viatura *) (arg);
	int fd = open(FIFOO, O_RDONLY);
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){
			//enviar para trás "fechado"
		}
		else {
			//criar thread arrumador e passar vehicle
		}

	}
	close(fd);
	return NULL;
}

int main(int argc, char ** argv){
	
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <N_LUGARES> <T_ABERTURA>\n", argv[0]);
		exit(1);
	}
	
	int f_places = atoi(argv[1]);
	//int fp = f_places;
	int duration = atoi(argv[2]);
	
	if(f_places <= 0 || duration <= 0)
	{
		fprintf(stderr, "Illegal arguments");
		exit(2);
	}
	
	//double elapsedTime = 0;
	//clock_t start = clock(), curr_time;
	
	int fdN, fdS, fdE, fdO;
	pthread_t tid_n, tid_s, tid_e, tid_o;
	
	mkfifo(FIFON, 0600);
	pthread_create(&tid_n, NULL, tcontrollerN, NULL);
	fdN = open(FIFON, O_WRONLY);
	
	mkfifo(FIFOS, 0600);
	pthread_create(&tid_s, NULL, tcontrollerS, NULL);
	fdS = open(FIFOS, O_WRONLY);
	
	mkfifo(FIFOE, 0600);
	pthread_create(&tid_e, NULL, tcontrollerE, NULL);
	fdE = open(FIFOE, O_WRONLY);
	
	mkfifo(FIFOO, 0600);
	pthread_create(&tid_o, NULL, tcontrollerO, NULL);
	fdO = open(FIFOO, O_WRONLY);
	
	sleep(duration);
	
	Viatura * vehicle_stop = (Viatura*)malloc(sizeof(Viatura));
	vehicle_stop->id= -1;
	
	write(fdN, &vehicle_stop, sizeof(Viatura));
	close(fdN);
	unlink(FIFON);
	
	write(fdS, &vehicle_stop, sizeof(Viatura));
	close(fdS);
	unlink(FIFOS);
	
	write(fdE, &vehicle_stop, sizeof(Viatura));
	close(fdE);
	unlink(FIFOE);
	
	write(fdO, &vehicle_stop, sizeof(Viatura));
	close(fdO);
	unlink(FIFOO);
	
	pthread_exit(NULL);
}