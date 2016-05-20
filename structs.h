#ifndef STRUCT_H
#define STRUCT_H

#define FIFOPN "/tmp/fifoN"
#define FIFOPS "/tmp/fifoS"
#define FIFOPE "/tmp/fifoE"
#define FIFOPO "/tmp/fifoO"
#define MAX_LENGHT 5000
#define SAIU_PARQUE		0
#define ENTROU_PARQUE	1
#define PARQUE_CHEIO	2
#define PARQUE_ENCERROU 3
#define TPS sysconf(_SC_CLK_TCK)	//ticks per second

#define CONTR_N_ID	0
#define CONTR_S_ID	1
#define CONTR_E_ID	2
#define CONTR_O_ID	3

typedef struct
{
	char direccao; //accesso do parque para onde se vai dirigir
	int tempo; //quanto tempo irá estar estacionada
	int id; //número identificador da viatura (único)
} Viatura;


void mysleep(clock_t dur){
    clock_t start, curr_time;
    
    start = curr_time = clock();

    while ((curr_time - start) < (dur * (1000000/TPS))){
		curr_time = clock();
	}
}

#endif