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
sem_t * sem;

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
	
	if (fp <= 0){//free places
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
	char * fifo_portao = (char *) arg;
	int fd, closed = 0;
	Viatura * vehicle = (Viatura*)malloc(sizeof(Viatura));
	char info;
	printf("aaaaaaaa\n");
	if((fd = open(fifo_portao, O_RDONLY))==-1)
	{
		perror(fifo_portao);
		//free(v);
		unlink(fifo_portao);
		close(fd);
		exit(4);
	}printf("bbbbbbbb\n");
	
	while (read(fd, &vehicle, sizeof(Viatura)) != 0){//a partir daqui recebe comunicações dos geradores. se receber do portao é só para dizer que fechou
		if (vehicle->id == -1){
			closed = 1;
		}
		else if (closed){
			//enviar para trás "fechado"
			int fd_gerador;
			
			char private_fifo[MAX_LENGHT];
			sprintf(private_fifo, "/tmp/viatura%d", vehicle->id);
			
			if((fd_gerador = open(private_fifo, O_WRONLY)) == -1){
				perror(private_fifo);
				//free(vehicle);
				unlink(private_fifo);
				close(fd_gerador);
				exit(4);
			}
			
			info = PARQUE_ENCERROU;
			if (write(fd_gerador, &info, sizeof(char) ) == -1 ){
				perror(private_fifo);
				//free(vehicle);
				unlink(private_fifo);
				close(fd_gerador);
				//sem_post(sem);
				exit(3);
			}
			printf("veiculo %d tentou entrar, parque cheio\n", vehicle->id);
			
		}
		else {
			//criar thread arrumador e passar2016 vehicle
			
			pthread_t tid;
			
			if(pthread_create(&tid, NULL , tarrumador , vehicle)){
				printf("Error Creating Thread!\n");
				exit(3);
			}
			printf("veiculo %d agora ao cargo do thread arrumador\n", vehicle->id);
			pthread_detach(tid);
		}
		//unlink(private_fifo);
	}
	//unlink(fd_gerador);
	close(fd);
	unlink(fifo_portao);
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
	
	sem_unlink("/semaphore");
	//criar semaforo
	if((sem = sem_open("/semaphore",O_CREAT,0600,1)) == SEM_FAILED)
	{
		perror("WRITER failure in sem_open()");
		//unlink(private_fifo);
	    //free(v);
	    exit(3);
	}printf("000000\n");
	
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
	
	printf("sleeping\n");
	sleep(duration);
	
	//sem_post(sem);
	int * sem_val;
	sem_getvalue(sem, sem_val);
	printf("sem: %d\n", *sem_val);
	printf("before sem_wait in parque\n");
	sem_wait(sem);
	printf("after sem_wait in parque\n");
	
	Viatura * vehicle_stop = (Viatura*)malloc(sizeof(Viatura));
	vehicle_stop->id= -1;
	
	write(fdN, &vehicle_stop, sizeof(Viatura));
	close(fdN);
	
	write(fdS, &vehicle_stop, sizeof(Viatura));
	close(fdS);
	
	write(fdE, &vehicle_stop, sizeof(Viatura));
	close(fdE);
	
	write(fdO, &vehicle_stop, sizeof(Viatura));
	close(fdO);
	
	sem_post(sem);
	sem_close(sem);
	sem_unlink("/semaphore");
	
	printf("Parque esta agora fechado\n");
	
	pthread_exit(NULL);
}