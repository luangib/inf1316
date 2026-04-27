#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

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
    printf("Enviando Timer (IRQ0) e ocasionalmente I/O (IRQ1)...\n\n");

    int ciclos = 0;

    while (1) {
        sleep(2); 
        printf("[CONTROLLER] Disparando IRQ0 (Timer) para PID %d\n", kernel_pid);
        kill(kernel_pid, SIGALRM);
        
        ciclos++;
        
        // A cada 5 ciclos (10 segundos), simula que o Dispositivo D1 terminou
        if (ciclos % 5 == 0) {
            printf("\n[CONTROLLER] *** Disparando IRQ1 (Dispositivo D1 terminou!) ***\n\n");
            kill(kernel_pid, SIGUSR2); // SIGUSR2 será o nosso IRQ1
        }
    }

    return 0;
}