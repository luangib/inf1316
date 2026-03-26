// ex4.c

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t filho1, filho2;

    // Criando o primeiro filho
    filho1 = fork();
    if (filho1 == 0) {
        while(1) {
            printf("Filho 1 rodando... (PID: %d)\n", getpid());
            sleep(1);
        }
    }

    // Criando o segundo filho
    filho2 = fork();
    if (filho2 == 0) {
        while(1) {
            printf("Filho 2 rodando... (PID: %d)\n", getpid());
            sleep(1);
        }
    }

    // --- CÓDIGO DO PAI (ESCALONADOR) ---
    
    // Inicialmente, paramos os dois para começar o controle
    kill(filho1, SIGSTOP);
    kill(filho2, SIGSTOP);

    int trocas = 0;
    pid_t atual = filho1;
    pid_t espera = filho2;

    printf("\n[PAI] Iniciando escalonamento (10 trocas)...\n\n");

    while (trocas < 10) {
        printf("\n--- Troca %d ---\n", trocas + 1);
        
        // 1. Manda quem está esperando rodar
        printf("[PAI] Acordando processo %d\n", atual);
        kill(atual, SIGCONT);
        
        // 2. Deixa ele rodar por um tempo (fatia de tempo/quantum)
        sleep(2); 

        // 3. Manda ele parar para dar a vez ao outro
        printf("[PAI] Pausando processo %d\n", atual);
        kill(atual, SIGSTOP);

        // 4. Alterna quem é o próximo
        pid_t temp = atual;
        atual = espera;
        espera = temp;

        trocas++;
    }

    // Fim das 10 trocas: Hora do "limpa"
    printf("\n[PAI] 10 trocas atingidas. Encerrando filhos...\n");
    kill(filho1, SIGKILL);
    kill(filho2, SIGKILL);

    // Espera os processos serem removidos da tabela do SO
    wait(NULL);
    wait(NULL);

    printf("[PAI] Fim do programa.\n");
    return 0;
}