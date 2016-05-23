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

sem_t * sem;

void * tviatura(void * arg)
{	
	clock_t initial = times(NULL);	
	Viatura *v = (Viatura *) (arg);
	printf("thread viatura %d\n", v->id);
	char private_fifo[MAX_LENGHT];
	sprintf(private_fifo, "/tmp/viatura%d", v->id);
	if(mkfifo(private_fifo,S_IRWXU)!=0)
	{
		printf("ERROR FIFO\n");
	}
FILE * gerador_log = fopen(LOG_GERADOR,"a");
	if((sem = sem_open("/semaphore", 0)) == SEM_FAILED)
	{
		perror("WRITER failure in sem_open()");
		//unlink(private_fifo);
	    //free(&v);
	    exit(3);
	}
	
	sem_wait(sem);
	//criar fifo escrita
	int write_to_fifo;

	char write_fifo[MAX_LENGHT];
	sprintf(write_fifo,"/tmp/fifo%c",v->direccao);
	printf("Direcção: %s\n",write_fifo);
	if((write_to_fifo=open(write_fifo,O_WRONLY|O_NONBLOCK))<1)
	{
		printf("ERROR OPEN\n");
		unlink(private_fifo);
		close(write_to_fifo);
	    	sem_post(sem);
	    	exit(3);
	}
		
	//escrever para o fifo
	if( write( write_to_fifo, v, sizeof(Viatura) ) == -1 )
	{
		printf("WRITE ERRO\n");
	    //free(&v);
	    unlink(private_fifo);
	    close(write_to_fifo);
	    sem_post(sem);
	    exit(3);
	}
	close(write_to_fifo);
	sem_post(sem);
	sem_close(sem);
	
	//abrir fifo de leitura
	int read_from_fifo;
	if((read_from_fifo=open(private_fifo,O_RDONLY))==-1)
	{
		perror(private_fifo);
		//free(&v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(4);
	}
	//ler do fifo
	char info_from_park;
	if(read(read_from_fifo,&info_from_park,sizeof(char))==-1)
	{
		printf("ERROR. READ FROM FIFO\n");
		//free(&v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(4);
	}
	printf("info: %d\n", info_from_park);
	if(info_from_park==SAIU_PARQUE)
	{
		printf("viatura saiu do parque\n");
		fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ;      %d ; saída!\n",1,v->id,v->direccao,v->tempo,(int)(times(NULL)-initial));
		
	}
	if(info_from_park==PARQUE_CHEIO)
	{
		printf("parque cheio\n");
		fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ;      ? ; cheio!\n",1,v->id,v->direccao,v->tempo);
		
	}
	if(info_from_park==PARQUE_ENCERROU)
	{
		printf("parque fechou\n");
	}
	if(info_from_park==ENTROU_PARQUE)
	{
		printf("viatura entrou parque\n");
		fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ;      ? ; entrou\n",1,v->id,v->direccao,v->tempo);
	}
	free(v);
	unlink(private_fifo);
	close(read_from_fifo);
	return NULL;
}

int main (int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s <T_GERACAO> <U_RELOGIO>\n", argv[0]);
		exit(1);
	}

	double t_geracao = (double) atoi(argv[1]);
	int u_relogio = atoi(argv[2]);

	if(t_geracao <= 0 || u_relogio <= 0)
	{
		fprintf(stderr, "Illegal arguments");
		exit(2);
	}

	FILE * gerador_log = fopen(LOG_GERADOR,"w");
	fprintf(gerador_log, "t(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n");
	fclose(gerador_log);
	
	int id_viatura=0;
	double elapsedTime = 0;
	srand(time(NULL));
	clock_t start = clock(), curr_time;

	while(elapsedTime < t_geracao)
	{
		//Viatura v;
		Viatura * v = (Viatura*)malloc(sizeof(Viatura));
		
		int random_access = rand() % 4;
		//criar a direcção
		if(random_access==0)
		{
			v->direccao='N';
		}
		else if(random_access==1)
		{
			v->direccao='S';
		}
		else if(random_access==2)
		{
			v->direccao='E';
		}
		else
		{
			v->direccao='O';
		}
		
		//criar o identificador único
		v->id=id_viatura++;
		//gerar o tempo de estacionamento
		v->tempo=(rand()%10 + 1 )* u_relogio;
		
		int random = rand() % 10;
		
		if(random < 2) //20%
		{
			//esperar durante duas unidades de tempo
			mysleep(2 * u_relogio);
		}
		else if(random < 5) //30%
		{
			//esperar durante uma unidade de tempo
			mysleep(u_relogio);
		}
		//não é necessária uma condição pois acção a realizar é nula
		
		
		//criar thread para a viatura
		pthread_t tid;
		
		if(pthread_create(&tid, NULL , tviatura , v))
		{
		    printf("Error Creating Thread!\n");
		    exit(3);
		}
		pthread_detach(tid);
		
		
		
		curr_time = clock();
		elapsedTime = (curr_time - start) / (double) CLOCKS_PER_SEC;
		printf("time:%f\n", elapsedTime);
	}
	
	pthread_exit(NULL);
}
