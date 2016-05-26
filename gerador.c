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
clock_t main_thread;

void * tviatura(void * arg)
{	
	clock_t inicial=times(NULL);	
		
	Viatura *v = (Viatura *) (arg);	
	
	//Variáveis	
	char private_fifo[MAX_LENGHT];	
	char write_fifo[MAX_LENGHT];	
	
	//Inicialização 
	sprintf(private_fifo, "/tmp/viatura%d", v->id);
	sprintf(write_fifo,"/tmp/fifo%c",v->direccao);

	//Criar FIFO para a viatura
	unlink(private_fifo);
	if(mkfifo(private_fifo,S_IRWXU)!=0)
	{
		perror(private_fifo);
		free(v);
		exit(1);
	}
	//Abrir gerador.log
	FILE * gerador_log = fopen(LOG_GERADOR,"a");
	if(gerador_log==NULL)
	{
		printf("Error open gerador.log!\n");
		exit(2);
	}
	//Abrir semáforo
if((sem = sem_open("/semaphore", 0/*0,S_IRWXU,0)*/)) == SEM_FAILED)
	{
		printf("Error in sem_open()\n");
		unlink(private_fifo);
	   	free(v);
	    	exit(3);
	}
	//Fazer sem_wait
	sem_wait(sem);
	//Abrir FIFO para escrita
	int write_to_fifo=0;
	if((write_to_fifo=open(write_fifo,O_WRONLY | O_NONBLOCK))<1)
	{
		printf("error on open %s\n", strerror(errno));
		unlink(write_fifo);
		close(write_to_fifo);
	    sem_post(sem);
		free(v);
	    exit(4);
	}		
	//Escrever para o FIFO
	if( write( write_to_fifo, v, sizeof(Viatura) ) == -1 )
	{
	    printf("Error writing to FIFO\n");
	    free(v);
	    unlink(private_fifo);
	    close(write_to_fifo);
	    sem_post(sem);
	    exit(5);
	}
	//Fechar FIFO de escrita
	close(write_to_fifo);
	sem_post(sem);
	//Abrir FIFO de leitura
	int read_from_fifo;
	if((read_from_fifo=open(private_fifo,O_RDONLY))==-1)
	{
		perror(private_fifo);
		free(v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(6);
	}
	//Ler do FIFO
	int info_from_park;
	/*if(read(read_from_fifo,&info_from_park,sizeof(int))==-1)
	{
		printf("Error readind from FIFO\n");
		free(v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(7);
	}*/
	
	while (read(read_from_fifo, &info_from_park, sizeof(int)) != 0){
		
		//Processar informação vinda do FIFO
		if(info_from_park==SAIU_PARQUE)
		{
			printf("viatura saiu do parque\n");
			fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ; %6d ; saída!\n",(int)(times(NULL)-main_thread),v->id,v->direccao,v->tempo,(int)(times(NULL)-inicial));

		}
		if(info_from_park==PARQUE_CHEIO)
		{
			printf("parque cheio\n");
			fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ;      ? ; cheio!\n",(int)(times(NULL)-main_thread),v->id,v->direccao,v->tempo);

		}
		if(info_from_park==PARQUE_ENCERROU)
		{
			printf("parque fechou\n");
		}
		if(info_from_park==ENTROU_PARQUE)
		{
			printf("viatura entrou parque\n");
			fprintf(gerador_log,"%8d ; %7d ; %6c ; %10d ;      ? ; entrou\n",(int)(times(NULL)-main_thread),v->id,v->direccao,v->tempo);
		}
		
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
		exit(7);
	}

	double t_geracao = (double) atoi(argv[1]);
	int u_relogio = atoi(argv[2]);

	if(t_geracao <= 0 || u_relogio <= 0)
	{
		fprintf(stderr, "Illegal arguments");
		exit(8);
	}

	FILE * gerador_log = fopen(LOG_GERADOR,"w");
	fprintf(gerador_log, "t(ticks) ; id_viat ; destin ; t_estacion ; t_vida ; observ\n");
	fclose(gerador_log);
	main_thread = times(NULL);
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
		    exit(9);
		}
		pthread_detach(tid);
		
		
		
		curr_time = clock();
		elapsedTime = (curr_time - start) / (double) CLOCKS_PER_SEC;
		//printf("time:%f\n", elapsedTime);
	}
	
	pthread_exit(NULL);
}
