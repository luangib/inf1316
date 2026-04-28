#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h> // Necessário para srand() e time()

int main() {
    printf("InterControllerSim iniciado.\n");
    printf("Procurando o arquivo kernel.pid...\n");

    FILE *arquivo_pid = fopen("kernel.pid", "r");
    if (arquivo_pid == NULL) {
        printf("[ERRO] Nao achei o arquivo kernel.pid. O Kernel esta rodando?\n");
        exit(1);
    }

    pid_t kernel_pid;
    fscanf(arquivo_pid, "%d", &kernel_pid);
    fclose(arquivo_pid);

    printf("=> KernelSim encontrado! (PID: %d)\n", kernel_pid);
    printf("Iniciando geracao de IRQs a cada 500ms (Randômico para I/O)...\n\n");

    // Inicializa a semente randômica usando o relógio do sistema
    srand(time(NULL));

    while (1) {
        // Pausa por 500 milissegundos (500.000 microssegundos)
        usleep(500000); 
        
        // 1. IRQ0: Timer (Ocorre SEMPRE a cada 500ms)
        kill(kernel_pid, SIGALRM);
        // printf("[CONTROLLER] Disparando IRQ0 (Timer) para PID %d\n", kernel_pid); // Descomente se quiser ver cada batimento
        
        // 2. IRQ1: Dispositivo D1 terminou (10% de probabilidade)
        if ((rand() % 100) < 10) {
            printf("\n[CONTROLLER] *** Disparando IRQ1 (Dispositivo D1 terminou!) ***\n");
            kill(kernel_pid, SIGUSR2); 
        }

        // 3. IRQ2: Dispositivo D2 terminou (5% de probabilidade)
        if ((rand() % 100) < 5) {
            printf("\n[CONTROLLER] *** Disparando IRQ2 (Dispositivo D2 terminou!) ***\n");
            kill(kernel_pid, SIGURG); // Usaremos SIGURG para representar o IRQ2
        }
    }

    return 0;
}