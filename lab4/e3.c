#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;

    // Cria o pipe de comunicação
    if (pipe(fd) < 0) {
        perror("Erro ao criar o pipe");
        exit(1);
    }

    // Cria o processo filho
    pid = fork();

    if (pid < 0) {
        perror("Erro no fork");
        exit(1);
    }

    if (pid == 0) {
        // PROCESSO FILHO: vai executar o 'ps'
        
       
        close(fd[0]);
        
        if (dup2(fd[1], 1) == -1) {
            perror("Erro no dup2 do filho");
            exit(1);
        }
        
       
        close(fd[1]);

       
        execlp("ps", "ps", NULL);
        
        
        perror("Erro ao executar o ps");
        exit(1);
        
    } else {
        // PROCESSO PAI: vai executar o 'wc'
        
       
        close(fd[1]);
        
       
        if (dup2(fd[0], 0) == -1) {
            perror("Erro no dup2 do pai");
            exit(1);
        }
        
        execlp("wc", "wc", NULL);
        
        
        perror("Erro ao executar o wc");
        exit(1);
    }

    return 0;
}