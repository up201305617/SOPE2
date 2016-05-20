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


int f_places, fp;

void * tarrumador(void * arg){
	//arruma
	Viatura *vehicle = (Viatura *) (arg);
	int fd;
	char info;
	
	char private_fifo[MAX_LENGHT];
	sprintf(private_fifo, "/tmp/viatura%d", vehicle->id);

	if((fd = open(private_fifo, O_WRONLY)) == -1){
		perror(private_fifo);
		free(vehicle);
		unlink(private_fifo);
		close(fd);
		exit(4);
	}
	
	if (fp <= 0){
		info = PARQUE_CHEIO;
		if (write(fd, &info, sizeof(char) ) == -1 ){
			perror(private_fifo);
			free(vehicle);
			unlink(private_fifo);
			close(fd);
			//sem_post(sem);
			exit(3);
		}
	}
	else {
		info = ENTROU_PARQUE;
		if (write(fd, &info, sizeof(char) ) == -1 ){
			perror(private_fifo);
			free(vehicle);
			unlink(private_fifo);
			close(fd);
			//sem_post(sem);
			exit(3);
		}	
		
		fp--;
		
		mysleep(vehicle->tempo);
		
		info = SAIU_PARQUE;
		if (write(fd, &info, sizeof(char) ) == -1 ){
			perror(private_fifo);
			free(vehicle);
			unlink(private_fifo);
			close(fd);
			//sem_post(sem);
			exit(3);
		}
		
		fp++;
	}
	
	return NULL;
}

void * tcontroller(void * arg){
	char * fifo_controller = (char *) arg;
	int fd, closed = 0;
	Viatura * vehicle = (Viatura*)malloc(sizeof(Viatura));
	
	if((fd = open(fifo_controller, O_RDONLY))==-1)
	{
		perror(fifo_controller);
		//free(v);
		unlink(fifo_controller);
		close(fd);
		exit(4);
	}
	
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){/*
			//enviar para trás "fechado"
			if( write( fd, v, sizeof(Viatura) ) == -1 ){
				//printf("fechado\n");
				free(v);
				unlink(private_fifo);
				close(private_fifo);
				sem_post(sem);
				exit(3);
			}*/
		}
		else {
			//criar thread arrumador e passar vehicle
			
			pthread_t tid;
			
			if(pthread_create(&tid, NULL , tarrumador , vehicle)){
				printf("Error Creating Thread!\n");
				exit(3);
			}			
			pthread_detach(tid);
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
	
	f_places = atoi(argv[1]);
	fp = f_places;
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
	
	mkfifo(FIFOPN, 0600);
	pthread_create(&tid_n, NULL, tcontroller, FIFOPN);
	fdN = open(FIFOPN, O_WRONLY);
	
	mkfifo(FIFOPS, 0600);
	pthread_create(&tid_s, NULL, tcontroller, FIFOPS);
	fdS = open(FIFOPS, O_WRONLY);
	
	mkfifo(FIFOPE, 0600);
	pthread_create(&tid_e, NULL, tcontroller, FIFOPE);
	fdE = open(FIFOPE, O_WRONLY);
	
	mkfifo(FIFOPO, 0600);
	pthread_create(&tid_o, NULL, tcontroller, FIFOPO);
	fdO = open(FIFOPO, O_WRONLY);
	
	sleep(duration);
	
	Viatura * vehicle_stop = (Viatura*)malloc(sizeof(Viatura));
	vehicle_stop->id= -1;
	
	write(fdN, &vehicle_stop, sizeof(Viatura));
	close(fdN);
	unlink(FIFOPN);
	
	write(fdS, &vehicle_stop, sizeof(Viatura));
	close(fdS);
	unlink(FIFOPS);
	
	write(fdE, &vehicle_stop, sizeof(Viatura));
	close(fdE);
	unlink(FIFOPE);
	
	write(fdO, &vehicle_stop, sizeof(Viatura));
	close(fdO);
	unlink(FIFOPO);
	
	pthread_exit(NULL);
}