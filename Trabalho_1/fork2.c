#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
O programa possui um laço de repetição for com indice i que se repete três vezes.
Nesse laço, é criado um filho (B, C ou D) do nó raíz(A) em cada uma das repetições.
Dentro desse laço for inicial, para cada filho, existe outro laço for com indice j que executa de 1 até i. 
Assim, são criados respectivamente: 0, 1 (E) e 2 (F e G) filhos.
O pai/nó raiz(A), filho do bash, é printado apenas no primeiro laço (isso é controlado com o booleano print_pai)
O pai/nó raiz (A) tem um wait() para aguardar que seus filhos (B, C e D) terminem a execução
Cada filho do nó raíz (B, C e D) cria seu respectivo número de filhos (0, 1 e 2) e também aguarda com wait() até seus filhos (E, F, G) terminarem
O define NOS_LEVEL_1 determina o número de filhos que o nó raíz possui, que no caso são 3
*/

#define NOS_LEVEL_1 3

int nro_printf = 0, aux_nro_printf = 0;

int main(void){

  pid_t id_processo, id_processo2, id_pai; 
  int i, j; 
  bool print_pai=true; //variável utilizada para printar a raíz da árvore apenas uma vez

  for (i=0; i<NOS_LEVEL_1; i++){ 
    nro_printf++;
    id_processo = fork();

    if(id_processo < 0){ //Erro ao criar filho
      printf("Erro no fork\n"); 
      exit(1);
    }else if (id_processo > 0){ //É pai (A)
      if(print_pai){
        printf("[%d] sou o processo %d, filho de %d\n", nro_printf+aux_nro_printf, getpid(), getppid()); 
        print_pai = false;
      }
      wait(NULL); 
    } else if (id_processo == 0){ //É filho (B, C ou D)
      aux_nro_printf++;
      printf("[%d] sou o processo %d, filho de %d\n", nro_printf+aux_nro_printf, getpid(), getppid());
      id_pai = getpid(); //Guarda o PID de B, C e D para imprimir E, F e G

      for(j=1; j<=i; j++){
        nro_printf++;
        id_processo2 = fork();
        wait(NULL);

        if(id_processo2 < 0){ //Erro ao criar filho
          printf("Erro no fork\n"); 
          exit(1);
        }else if(getppid()==id_pai){ //É filho (E, F ou G)
          aux_nro_printf++;
          printf("[%d] sou o processo %d, filho de %d\n", nro_printf+aux_nro_printf, getpid(), getppid());
          break;
        }

      } 
      wait(NULL);
      break; 
    }
  } 
}