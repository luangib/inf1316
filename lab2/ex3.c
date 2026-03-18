#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>

int main() {
// Tentei simplificar o maximo possivel esse exercicio pq tava achando muito dificil. As modificações que eu fiz foram, os 4 processor vão procurar entre 16 ints(4 para cada) e todos os ints fora a chave estão vazio e o com a chave tem 1. Além disso só para saber quem achou e não achou imprimi o pid. O que muda não é o valor e sim qual int desses 16 estarão com 1. Foi oque achei mais facil simples de fazer. Pesquisei como randomizar(faz muito tempo que não usava)

// variaveis
int segmento, *vetor, i;
int alvo = 1;
int tamanho = 16;

//Processo de alocar memoria
segmento = shmget(IPC_PRIVATE, tamanho * sizeof(int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR );
vetor = (int *) shmat(segmento, 0, 0);

//Preparando vetor e sorteio
for(i = 0; i < tamanho; i++) vetor[i] = 0;

srand(time(NULL));
int sorte = rand() % tamanho;
vetor[sorte] = alvo; 

printf("O Numero estará na posicao %d.\n", sorte);

//Processo de Busca
printf("Iniciando da Busca");
for (i = 0; i < 4; i++) {
    if (fork() == 0) {

        int inicio = i * 4;
        int fim = inicio + 4;
        int achei = 0;

        for (int j = inicio; j < fim; j++) {
            if (vetor[j] == alvo) {
                printf("PID %d: Estavana posicao %d.\n", getpid(), j);
                achei = 1;
            }
        }

        if (!achei) {
            printf(" PID %d: Não achei (pos %d-%d).\n", getpid(), inicio, fim-1);
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