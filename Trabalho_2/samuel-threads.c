#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

double diffTimeSec(struct timespec t0, struct timespec t1) {
	return ((double) t1.tv_sec - t0.tv_sec) + ((double) (t1.tv_nsec - t0.tv_nsec) * 1e-9);
}

int aliquot_sum(int n){
	int soma = 0;
	for(int i=1; i<=n/2;i++){
		if(n % i == 0){
			soma+=i;
		}
	}
	return soma;
}

void ajusta_threads_worksize(int *threads_worksize, int threads, int worksize){
	int thread_worksize = worksize/threads;
	for(int i=0; i<threads; i++){
		threads_worksize[i] = thread_worksize; 
	}
	int i=0;
	int sobra_thread_worksize = worksize % threads;
	while(sobra_thread_worksize>0){
		threads_worksize[i] += 1;
		i++;
		sobra_thread_worksize--;
	}
}

typedef struct{
   int idThread;
   int threads;
   int worksize;
   int tWorksize;
   int iniWorksize;
   int defectivo;
   int abundante;
   int perfeito;
} t_Args;

void *chunks (void *arg){
	t_Args *args = (t_Args *) arg;
	int soma, defectivo, abundante, perfeito;

	for(int i=1; i<= args->tWorksize; i++){

		soma = aliquot_sum(args->iniWorksize);
		if(soma<args->iniWorksize){
			args->defectivo++;
		}else if(soma>args->iniWorksize){
			args->abundante++;
		}else{
			args->perfeito++;
		}
		args->iniWorksize++;
	}	

	return (void *)args;
}

void *esparsa (void *arg){
	t_Args *args = (t_Args *) arg;
	int soma, defectivo, abundante, perfeito;
	int i = args->idThread;

	while(i<=args->worksize){
		//printf("Sou a thread %d testando número %d\n", args->idThread, i);
		soma = aliquot_sum(i);
		if(soma<i){
			args->defectivo++;
		}else if(soma>i){
			args->abundante++;
		}else{
			args->perfeito++;
		}
		i += args->threads;
	}		

	return (void *)args;
}

double criaThreads(int threads, int worksize, bool tipo){
	struct timespec start, end;
	t_Args *arg;
	pthread_t tid[threads];
	int t;
	int worksize_thread_anterior = 0;
	int local_defectivo=0, local_abundante=0, local_perfeito=0;
  	
	int threads_worksize[threads];
	ajusta_threads_worksize(threads_worksize, threads, worksize);

	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	for(t=1; t<=threads; t++) {
   	arg = malloc(sizeof(t_Args));
    if (arg == NULL) {
    	printf("erro ao alocar memoria\n"); 
    	exit(-1);
    }
    arg->idThread = t; 
		arg->threads = threads;
		arg->worksize = worksize;
    arg->tWorksize = threads_worksize[t-1]; 
		arg->iniWorksize = worksize_thread_anterior + 1;
		arg->defectivo = 0;
		arg->abundante = 0;
		arg->perfeito = 0;
		worksize_thread_anterior = worksize_thread_anterior + arg->tWorksize;
    
		if(!tipo){
    	if (pthread_create(&tid[t], NULL, chunks, (void*) arg)) {
    		printf("erro ao criar a threas\n"); 
    		exit(-1);
    	}
		}else{
			if (pthread_create(&tid[t], NULL, esparsa, (void*) arg)) {
    		printf("erro ao criar a threas\n"); 
    		exit(-1);
    	}
		} 	
  }

	printf("\n                     [D]         [A]         [P]          [WTot]\n");
	for (t=1; t<=threads; t++) {
		void *returnValue;       

		if (pthread_join(tid[t], &returnValue)) {
       		printf("erro ao executar join\n"); 
	 		exit(-1); 
    	}
		t_Args *retorno = returnValue;
		printf("* Thread %d:     %8d    %8d    %8d        %8d\n", retorno->idThread, retorno->defectivo, retorno->abundante, retorno->perfeito, retorno->tWorksize);
		local_defectivo+=retorno->defectivo;
		local_abundante+=retorno->abundante;
		local_perfeito+=retorno->perfeito;
		free(retorno);
  	}
	printf("* TOTAL:        %8d    %8d    %8d        %8d\n", local_defectivo, local_abundante, local_perfeito, worksize);
	printf("---------------------------------");
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	return diffTimeSec(start, end);
}

double sequencial(int worksize) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);

	int defectivo=0, abundante=0, perfeito=0; 
	int soma;
	for(int i=1; i<=worksize; i++){
		soma = aliquot_sum(i);
		if(soma<i){
			defectivo++;
		}else if(soma>i){
			abundante++;
		}else{
			perfeito++;
		}
	}
	printf("---------------------------------\n");
	printf("                     [D]         [A]         [P]          [WTot]");
	printf("\n* Sequencial:   %8d    %8d    %8d        %8d", defectivo, abundante, perfeito, worksize);
	printf("\n---------------------------------");
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	return diffTimeSec(start, end);
}	

int main(int argc, char *argv[ ]) {

	if(argc!= 3){
		printf("Uso: ./samuel-threads <qtd_threads> <worksize>");
		exit(0);
	}

	int threads = atoi(argv[1]);
	int worksize = atoi(argv[2]);

	double tempo_sequencial = sequencial(worksize);
	double tempo_chunks = criaThreads(threads, worksize, 0);
	double tempo_esparsa = criaThreads(threads, worksize, 1);

	printf("\nTempo da classificação sequencial: %lf seg\n", tempo_sequencial);
	printf("Tempo da classificação com threads: %lf seg (distribuição por chunk)\n", tempo_chunks);
	printf("Tempo da classificação com threads: %lf seg (distribuição esparsa)\n", tempo_esparsa);

	return 0;
}