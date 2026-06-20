#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h> 

#define MAX_ITERATIONS 3000

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Erro: Faltam argumentos.\n");
        exit(1);
    }

    char *nome = argv[1]; 
    int write_fd = atoi(argv[2]); 
    int PC = 0; 

    srand(time(NULL) ^ getpid());

    // NOVO: Inicializa a pagina logica atual (0 a 15) fora do loop
    int mem = rand() % 16; 

    while(PC < MAX_ITERATIONS) {
        printf("Processo %s (PID: %d) executando... PC: %d, Mem: m%02d\n", nome, getpid(), PC, mem);
        
        // NOVO: Fase 3 - 40% de chance de acessar uma nova pagina de memoria
        if ((rand() % 100) < 40) {
            mem = rand() % 16; // Sorteia o indice da nova pagina
            char operacao = (rand() % 2 == 0) ? 'R' : 'W'; // Decide entre Leitura (R) ou Escrita (W)

            char mensagem[50];
            // Enviamos "M" como dispositivo para sinalizar ao Kernel que eh uma Syscall de Memoria
            sprintf(mensagem, "%s M %c m%02d %d", nome, operacao, mem, PC);
            
            write(write_fd, mensagem, strlen(mensagem) + 1);
            printf("\n>>> %s EXECUTOU SYSCALL DE MEMORIA: Op: %c | Mem: m%02d <<<\n", nome, operacao, mem);
            
            kill(getppid(), SIGUSR1); // Avisa o Kernel
            kill(getpid(), SIGSTOP);  // Bloqueia esperando a pagina ir para a RAM
        }
        // Se nao acessou nova pagina, roda a logica antiga de D1/D2
        else {
            int d = rand() % 100;
            if ((d + 1) < 15) { // 14% de chance
                char dispositivo[3];
                char operacao;
                
                if (rand() % 2 == 0) strcpy(dispositivo, "D1");
                else strcpy(dispositivo, "D2");
                
                int op_rand = rand() % 3;
                if (op_rand == 0) operacao = 'R';
                else if (op_rand == 1) operacao = 'W';
                else operacao = 'X';

                char mensagem[50];
                sprintf(mensagem, "%s %s %c m%02d %d", nome, dispositivo, operacao, mem, PC);
                
                write(write_fd, mensagem, strlen(mensagem) + 1);
                printf("\n>>> %s EXECUTOU SYSCALL DE I/O: Disp: %s | Op: %c <<<\n", nome, dispositivo, operacao);
                
                kill(getppid(), SIGUSR1);
                kill(getpid(), SIGSTOP); 
            }
        }
        
        PC++;
        usleep(500000);
    }
    
    printf("\nProcesso %s finalizou suas %d iteracoes com sucesso.\n", nome, MAX_ITERATIONS);
    return 0;
}