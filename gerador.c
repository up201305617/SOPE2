#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define FIFOPN "/tmp/fifoN"
#define FIFOPS "/tmp/fifoS"
#define FIFOPE "/tmp/fifoE"
#define FIFOPO "/tmp/fifoO"

struct
{
	char direccao;
	int id;
} Viatura;

void * tviatura(void * arg)
{
	pthread_t self = pthread_self();

	struct Viatura v = *(struct Viatura *) arg;

	if(pthread_detach(self)!=0)
	{
		printf("Error in thread %d",(int)self);
		exit(1);
	}

	return NULL;
}

int main (int int argc, char* argv[])
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

	srand(time(NULL));
	int elapsedTime;


	while(elapsedTime < t_geracao)
	{
		int random = rand % 10;

		if(random<5) //50%
		{

		}
		else if(random<8) //30%
		{

		}
		else //20%
		{

		}
	}
}
