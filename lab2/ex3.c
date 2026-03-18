#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define TAM 16

int main() {
// Tentei simplificar o maximo possivel esse exercicio pq tava achando muito dificil. As modificações que eu fiz foram, os 4 processor vão procurar entre 16 ints(4 para cada) e todos os ints fora a chave estão vazio e o com a chave tem 1. Além disso só para saber quem achou e não achou imprimi o pid. O que muda não é o valor e sim qual int desses 16 estarão com 1. Foi oque achei mais facil simples de fazer. Pesquisei como randomizar(faz muito tempo que não usava)

// variaveis
int segmento, *vetor, i;
int alvo = 1;

//Processo de alocar memoria
segmento = shmget(IPC_PRIVATE, TAM * sizeof(int), IPC_CREAT | 0666);
vetor = (int *) shmat(segmento, 0, 0);

//Preparando vetor e numero
for(i = 0; i < TAM; i++) vetor[i] = 0;

srand(time(NULL));
int pos_sorteada = rand() % TAM;
vetor[pos_sorteada] = alvo; 

printf("O Numero estará na posicao %d.\n", getpid(), pos_sorteada);

//Processo de Busca
printf("Iniciando da Busca");
for (i = 0; i < 4; i++) {
    if (fork() == 0) {

        int inicio = i * 4;
        int fim = inicio + 4;
        int achei = 0;

        for (int j = inicio; j < fim; j++) {
            if (vetor[j] == alvo) {
                printf("PID %d: Estavana posicao %d.\n", i, getpid(), alvo, j);
                achei = 1;
            }
        }

        if (!achei) {
            printf(" PID %d: Não achei (pos %d-%d).\n", i, getpid(), inicio, fim-1);
        }
        
        shmdt(vetor);
        exit(0); 
    }
}

for (i = 0; i < 4; i++) wait(NULL);

//Libera Memória
shmdt(vetor);
shmctl(segmento, IPC_RMID, NULL); 

printf("\nFim.\n");
return 0;
}