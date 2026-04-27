#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h> // <-- Adicione esta biblioteca

int main(int argc, char *argv[]) {
    if (argc < 3) exit(1);

    char *nome = argv[1]; 
    int write_fd = atoi(argv[2]); 
    int contador = 0;

    while(1) {
        printf("Processo %s (PID: %d) rodando...\n", nome, getpid());
        
        // Simulação: A2 pede o D1
        if (strcmp(nome, "A2") == 0 && contador == 3) {
            char mensagem[50];
            // Envia qual aplicação está pedindo (ex: A2) e o dispositivo
            sprintf(mensagem, "%s pediu D1", nome);
            
            write(write_fd, mensagem, strlen(mensagem) + 1);
            printf("\n>>> %s SOLICITOU USO DO DISPOSITIVO D1 <<<\n", nome);
            
            // TRAP: Acorda o Kernel avisando que tem mensagem no cano!
            kill(getppid(), SIGUSR1);
            
            // Pausa a si mesmo (o Kernel normalmente faria isso, mas simulamos aqui 
            // para a aplicação "congelar" enquanto espera o Disco)
            kill(getpid(), SIGSTOP); 
            
            contador = 0;
        }
        
        contador++;
        sleep(2); 
    }
    return 0;
}