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

#define FIFOPN "/tmp/fifoN"
#define FIFOPS "/tmp/fifoS"
#define FIFOPE "/tmp/fifoE"
#define FIFOPO "/tmp/fifoO"
#define MAX_LENGHT 5000
#define SAIU_PARQUE 0
#define PARQUE_CHEIO 1
#define PARQUE_ENCERROU 2

int id_viatura=0;
sem_t * sem;

typedef struct
{
	char direccao; //accesso do parque para onde se vai dirigir
	int tempo; //quanto tempo irá estar estacionada
	int id; //número identificador da viatura (único)
} Viatura;

void * tviatura(void * arg)
{
	Viatura *v = (Viatura *) (arg);
	//criar fifo privado de nome único - usar o id da viatura
	char private_fifo[MAX_LENGHT];
	sprintf(private_fifo, "/tmp/viatura%d", v->id);
	if(mkfifo(private_fifo,0660)!=0)
	{
		perror(private_fifo);
		free(v);
		exit(2);
	}
	//criar semaforo
	if((sem = sem_open("/semaphore",O_CREAT,0600,0)) == SEM_FAILED)
	{
		perror("WRITER failure in sem_open()");
		unlink(private_fifo);
	    free(v);
	    return NULL;
	}
	sem_wait(sem);
	//criar fifo escrita
	int write_to_fifo;
	switch (v->direccao)
	{
	     case 'N':
	    	 if((write_to_fifo = open(FIFOPN, O_WRONLY)) == -1)
	    	 {
	    		 perror(private_fifo);
	    		 unlink(private_fifo);
	    		 close(write_to_fifo);
	    		 sem_post(sem);
	    		 free(v);
	    		 return NULL;
	    	 }
	    	 break;
		 case 'S':
			 if((write_to_fifo = open(FIFOPS, O_WRONLY)) == -1)
			 {
				 perror(private_fifo);
				 unlink(private_fifo);
				 close(write_to_fifo);
				 sem_post(sem);
				 free(v);
				 return NULL;
			 }
	         break;
	     case 'E':
	    	 if((write_to_fifo = open(FIFOPE, O_WRONLY)) == -1)
			 {
				 perror(private_fifo);
				 unlink(private_fifo);
				 close(write_to_fifo);
				 sem_post(sem);
				 free(v);
				 return NULL;
			 }
	         break;
	     case 'O':
	    	 if((write_to_fifo = open(FIFOPO, O_WRONLY)) == -1)
			 {
				 perror(private_fifo);
				 unlink(private_fifo);
				 close(write_to_fifo);
				 sem_post(sem);
				 free(v);
				 return NULL;
			 }
	         break;
	}
	//escrever para o fifo
	if( write( write_to_fifo, v, sizeof(Viatura) ) == -1 )
	{
		printf("WRITE ERRO\n");
	    free(v);
	    unlink(private_fifo);
	    close(write_to_fifo);
	    sem_post(sem);
	    exit(3);
	}
	close(write_to_fifo);
	sem_post(sem);
	//abrir fifo de leitura
	int read_from_fifo;
	if((read_from_fifo=open(private_fifo,O_RDONLY))==-1)
	{
		perror(private_fifo);
		free(v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(4);
	}
	//ler do fifo
	char info_from_park;
	if(read(read_from_fifo,&info_from_park,sizeof(char))==-1)
	{
		printf("READ ERRO\n");
		free(v);
		unlink(private_fifo);
		close(read_from_fifo);
		exit(4);
	}

	if(info_from_park==SAIU_PARQUE)
	{
		printf("viatura saiu do parque\n");
	}
	if(info_from_park==PARQUE_CHEIO)
	{
		printf("parque cheio\n");
	}
	if(info_from_park==PARQUE_ENCERROU)
	{
		printf("parque fechou\n");
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

	if(t_geracao<=0 || u_relogio<=0)
	{
		fprintf(stderr, "Illegal arguments");
		exit(2);
	}
	
	clock_t start = clock(), curr_time;
	srand(time(NULL));
	double elapsedTime=0;

	while(elapsedTime < t_geracao)
	{
		Viatura* v = (Viatura*)malloc(sizeof(Viatura));

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

		iif(random < 2) //20%
		{
			//esperar durante duas unidades de tempo
			usleep(1000 * 10 * 2 * u_relogio);		// * 1000 porque usleep trabalha com microsegundos
			elapsedTime += 2 * u_relogio;
		}
		else if(random < 5) //30%
		{
			//esperar durante uma unidade de tempo
			usleep(1000 * 10 * u_relogio);
			elapsedTime += u_relogio;
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
		elapsedTime = (curr_time - start)/ (double) CLOCKS_PER_SEC;
	}
	pthread_exit(NULL);
	return 0;
}