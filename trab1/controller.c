#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>

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


    srand(time(NULL));

    while (1) {
        usleep(500000); 
        kill(kernel_pid, SIGALRM);
        
        if ((rand() % 100) < 10) {
            printf("\n[CONTROLLER] *** Disparando IRQ1 (Dispositivo D1 terminou!) ***\n");
            kill(kernel_pid, SIGUSR2); 
        }

        if ((rand() % 100) < 5) {
            printf("\n[CONTROLLER] *** Disparando IRQ2 (Dispositivo D2 terminou!) ***\n");
            kill(kernel_pid, SIGURG);
        }
    }

    return 0;
}