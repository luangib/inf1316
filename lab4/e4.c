#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid_leitor1, pid_leitor2;

    if (pipe(fd) < 0) {
        perror("Erro ao criar a pipe");
        exit(1);
    }

    /* ---------------- CRIANDO O PRIMEIRO LEITOR ---------------- */
    pid_leitor1 = fork();
    if (pid_leitor1 < 0) {
        perror("Erro no fork do Leitor 1");
        exit(1);
    }

    if (pid_leitor1 == 0) {
        close(fd[1]); 
        char buffer[50];
        
        
        while (read(fd[0], buffer, sizeof(buffer)) > 0) { 
            printf("[Leitor 1 - PID %d] consumiu: %s\n", getpid(), buffer);
            sleep(2); // Dorme o dobro do tempo do escritor
        }
        
        close(fd[0]);
        exit(0);
    }

    /* ---------------- CRIANDO O SEGUNDO LEITOR ---------------- */
    pid_leitor2 = fork();
    if (pid_leitor2 < 0) {
        perror("Erro no fork do Leitor 2");
        exit(1);
    }

    if (pid_leitor2 == 0) {
        close(fd[1]);
        char buffer[50];
        
        while (read(fd[0], buffer, sizeof(buffer)) > 0) {
            printf("[Leitor 2 - PID %d] consumiu: %s\n", getpid(), buffer);
            sleep(2); // Dorme o dobro do tempo do escritor
        }
        
        close(fd[0]);
        exit(0);
    }

    /* ---------------- PROCESSO PAI (ESCRITOR) ---------------- */
    
    close(fd[0]);
    char msg[50];

    printf("Iniciando transmissões...\n\n");

    // 6 mensagens no total
    for (int i = 1; i <= 6; i++) {
        snprintf(msg, sizeof(msg), "Dado #%d", i);
        
        // Escreve na pipe passando o tamanho exato + 1 para pegar o '\0'
        write(fd[1], msg, strlen(msg) + 1); 
        printf("[Escritor - PID %d] produziu: %s\n", getpid(), msg);
        
        sleep(1); // Escritor dorme metade do tempo dos leitores
    }

   
    close(fd[1]); 

    
    wait(NULL);
    wait(NULL);
    
    printf("\nTodos os processos finalizaram.\n");

    return 0;
}