#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
O programa possui um laço de repetição for com indice i que se repete duas vezes.
Nesse laço, é criado um filho (B ou C) do nó raíz(A) em cada uma das repetições.
Dentro desse laço for inicial, para cada filho, existe outro laço for com indice j que se repete duas vezes. 
Assim, são criados dois filhos para cada filho (D e E para B) (F e G para C).
O pai/nó raiz(A), filho do bash, é printado apenas no primeiro laço (isso é controlado com o booleano print_pai).
O pai/nó raiz (A) tem um wait() para aguardar que seus filhos (B e C) terminem a execução.
Cada filho do nó raíz (B e C) cria dois filhos e também aguarda com wait() até seus filhos (D, E, F, G) terminarem.
O define NOS_LEVEL_1 determina o número de filhos que o nó raíz possui, que no caso são 2.
O define NOS_LEVEL_2 determina o número de filhos que cada nó do nível 1 possui, que no caso são 2.
*/

#define NOS_LEVEL_1 2
#define NOS_LEVEL_2 2

int nro_printf = 0, aux_nro_printf = 0;

int main(void) {

  pid_t id_processo, id_processo2, id_pai; 
  int i, j; 
  bool print_pai=true; //variável utilizada para printar a raíz da árvore apenas uma vez

  for (i=1; i<=NOS_LEVEL_1; i++){ 
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
    } else if (id_processo == 0){ 
      aux_nro_printf++;
      printf("[%d] sou o processo %d, filho de %d\n", nro_printf+aux_nro_printf, getpid(), getppid());
      id_pai = getpid(); //Guarda o PID de B e C para imprimir D e E

      for(j=0; j<NOS_LEVEL_2; j++){
        nro_printf++;
        id_processo2 = fork();
        wait(NULL);

        if(id_processo2 < 0){
          printf("Erro no fork\n"); 
          exit(1);
        }else if(getppid()==id_pai){ //Testa se é filho de B ou C para imprimir D e E
          aux_nro_printf++;
          printf("[%d] sou o processo %d, filho de %d\n", nro_printf+aux_nro_printf+j, getpid(), getppid());
          break;
        }

      } 
      wait(NULL);
      break; 
    }
  }
}