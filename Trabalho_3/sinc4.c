#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

//struct da fila
typedef struct{
	int capacidade;
	int *dados;
	int primeiro;
	int ultimo;
	int nItens; 
} Fila;

//declara fila para 2 buffers
Fila *buffer_b1, *buffer_b2;

//inicializa fila
void inicializaFila(Fila *fila, int max) { 
	fila->capacidade = max;
	fila->dados = (int*) malloc (fila->capacidade * sizeof(int));
	fila->primeiro = 0;
	fila->ultimo = -1;
	fila->nItens = 0; 
}

//inserção na fila
void inserir(Fila *fila, int v) {
	if(fila->ultimo == fila->capacidade-1){
		fila->ultimo = -1;
	}	
	fila->ultimo++;
	fila->dados[fila->ultimo] = v; // incrementa ultimo e insere
	fila->nItens++; // mais um item inserido
}

//remoção da fila
int remover(Fila *fila) { // pega o item do começo da fila
	int temp = fila->dados[fila->primeiro++];
	if(fila->primeiro == fila->capacidade){
		fila->primeiro = 0;
	}
	fila->nItens--;  // um item retirado
	return temp;
}

//fila vazia
int estaVazia(Fila *fila) { // retorna verdadeiro se a fila está vazia
	return (fila->nItens==0);
}

//fila cheia
int estaCheia(Fila *fila) { // retorna verdadeiro se a fila está cheia
	return (fila->nItens == fila->capacidade);
}

//printa a fila (buffer)
void printarFila(Fila *fila){
    int cont, i;
    for (cont=0, i= fila->primeiro; cont < fila->nItens; cont++){
        printf("%d\t",fila->dados[i++]);
        if (i == fila->capacidade)
            i=0;
    }
    printf("\n\n");
}

//semaforos s1 e s2
sem_t pos_vazia_s1;
sem_t pos_ocupada_s1;
sem_t pos_vazia_s2;
sem_t pos_ocupada_s2;

//struct de parâmetros
typedef struct{
   int max;
   int qtd_adm;
   int qtd_div;
   int index_arq;
} t_Args;

//thread de criação de arquivos da administração
void* arq_adm(void *v){
	
	sem_wait(&pos_vazia_s1);
	t_Args *args = (t_Args *) v;
	inserir(buffer_b1, args->index_arq);
	printf("\nArquivo ADM %d adicionado ao buffer_b1\nBuffer b1: \t", args->index_arq);
	printarFila(buffer_b1);
	sem_post(&pos_ocupada_s1);
	sleep(random() % 3);

	return NULL;
}

//thread de criação de arquivos diversos
void* arq_div(void *v){

	sem_wait(&pos_vazia_s2);
	t_Args *args = (t_Args *) v;
	inserir(buffer_b2, args->index_arq);
	printf("\nArquivo DIV %d adicionado ao buffer_b2\nBuffer b2: \t", args->index_arq);
	printarFila(buffer_b2);
	sem_post(&pos_ocupada_s2);
	sleep(random() % 3);

	return NULL;
}

//thread da impressora (consumidor)
void* impressora(void *v){

	t_Args *args = (t_Args *) v;
	int i, x=1;

	for(i=0; i < (args->qtd_adm + args->qtd_div); i++){
		if(estaVazia(buffer_b1)){
			x=3;
		}
		if(estaVazia(buffer_b2)){
			x=2;
		}
		if(x==1 || x==2){
			sem_wait(&pos_ocupada_s1);
			printf("\nImpressão ADM, item = %d\nBuffer b1: \t", remover(buffer_b1));
			printarFila(buffer_b1);
			x++;
			sem_post(&pos_vazia_s1);
			sleep(random() % 3);	
		}else if(x==3){
			sem_wait(&pos_ocupada_s2);
			printf("\nImpressão DIV, item = %d\nBuffer b2: \t", remover(buffer_b2));
			printarFila(buffer_b2);
			x=1;
			sem_post(&pos_vazia_s2);
			sleep(random() % 3);			
		}
	}
	return NULL;
}

//inicializa args
void inicializaArgs(t_Args *args, int max, int qtd_adm, int qtd_div, int flag){
	if(flag==1){
		for(int i=0; i<qtd_adm; i++){
			args[i].max = max;
			args[i].qtd_adm = qtd_adm;
			args[i].qtd_div = qtd_div;
			args[i].index_arq = i+1;
		}
	}else{
		for(int i=0; i<qtd_div; i++){
			args[i].max = max;
			args[i].qtd_adm = qtd_adm;
			args[i].qtd_div = qtd_div;
			args[i].index_arq = i+1;
		}
	}	 
}

int main(int argc, char *argv[ ]) {

	//teste de parâmetros argc
	if(argc!= 4){
		printf("Uso: ./samuel-threads <max> <qtd_adm> <qtd_diversos>");
		exit(0);
	}
	if(atoi(argv[1])==0){
		printf("Capacidade do armazenamento dos buffers deve ser maior do que 0");
		exit(0);
	}

	//pega entradas do argv e guarda nas variaveis
	int max, qtd_adm, qtd_div;
	max = atoi(argv[1]);
	qtd_adm = atoi(argv[2]);
	qtd_div = atoi(argv[3]);

	//aloca argumentos
	t_Args *args_b1;
	t_Args *args_b2;
	args_b1 = (t_Args*) malloc(qtd_adm * sizeof(t_Args));
	args_b2 = (t_Args*) malloc(qtd_div * sizeof(t_Args));
	inicializaArgs(args_b1, max, qtd_adm, qtd_div, 1);
	inicializaArgs(args_b2, max, qtd_adm, qtd_div, 2);

	//inicializa filas
	buffer_b1 = (Fila*) malloc (sizeof(Fila));
	buffer_b2 = (Fila*) malloc (sizeof(Fila));
	inicializaFila(buffer_b1, max);
	inicializaFila(buffer_b2, max);

	//declaração dos vetores de threads
	pthread_t thr_arq_adm[qtd_adm], thr_arq_div[qtd_div], thr_impressora;

	//semáforo buffer_b1 e buffer_b2
	sem_init(&pos_vazia_s1, 0, max);
	sem_init(&pos_ocupada_s1, 0, 0);
	sem_init(&pos_vazia_s2, 0, max);
	sem_init(&pos_ocupada_s2, 0, 0);

	//cria thread de impressão da administração e diversos e a impressora	
	for(int i=0; i<qtd_div; i++){
		pthread_create(&thr_arq_div[i], NULL, arq_div, (void*) &args_b2[i]);
	}
	for(int i=0; i<qtd_adm; i++){
		pthread_create(&thr_arq_adm[i], NULL, arq_adm, (void*) &args_b1[i]);
	}
	pthread_create(&thr_impressora, NULL, impressora, (void*) &args_b1[0]);

	//pthread_join(thr_arq_adm, NULL);
	for(int i=0; i<qtd_adm; i++){
		pthread_join(thr_arq_adm[i], NULL);
	}
	for(int i=0; i<qtd_div; i++){
		pthread_join(thr_arq_div[i], NULL);
	}
	pthread_join(thr_impressora, NULL);

	//destrói semáforos s1 e s2
	sem_destroy(&pos_vazia_s1);
	sem_destroy(&pos_ocupada_s1);
	sem_destroy(&pos_vazia_s2);
	sem_destroy(&pos_ocupada_s2);

	//libera memória dos args e dos buffers
	free(buffer_b1);
	free(buffer_b2);
	free(args_b1);
	free(args_b2);
	
	return 0;
}