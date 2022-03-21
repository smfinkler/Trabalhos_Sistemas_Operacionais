#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
O programa utiliza uma função recursiva para criar uma hierarquia com n filhos.
O número de filhos é lido na main e passado como parâmetro na função.
A função recursiva recebe i=1 para começar a iteração e max com o valor informado para o número de filhos na hierarquia.
Na função recursiva, é feito fork e chamada novamente a função no filho até que a condição de parada seja atendida.
A condição de parada é o índice se tornar igual ao número de filhos da hierarquia.
O pai/nó raiz, filho do bash, foi ocultado e aparece apenas no print do seu filho, como pai.
*/

void cria_filhos(int i, int max, pid_t id_processo){
	if (i==max){
  	return;
  }else{
    id_processo = fork();
    wait(NULL);
    if(id_processo < 0){
      printf("Erro no fork\n"); 
      exit(1);
    }else if(id_processo==0){
      printf("i= %d, ID do processo: %d, ID do pai: %d, ID do filho: %d\n", i, getpid(), getppid(), getpid()+1);
      cria_filhos(i+1,  max, id_processo);
    }
  }      
}

int main(){
  pid_t id_processo;
	int n;
	printf("Informe o número de filhos: ");
  scanf("%d", &n);
  cria_filhos(1, n+1, id_processo);
	return 0;
}