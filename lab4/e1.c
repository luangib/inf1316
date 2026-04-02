#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int fd[2]; 
    pid_t pid;
    int nDadosTx, nDadosRx;
    
    const char mensagem_filho[] = "Olá Pai, mensagem enviada via pipe!";
    char buffer_pai[100];

    
    if (pipe(fd) < 0) { 
        puts("Erro ao abrir os pipes");
        exit(-1);
    }

    pid = fork();

    if (pid < 0) {
        perror("Erro no fork");
        exit(-1);
    }

    if (pid == 0) {
        /* PROCESSO FILHO (Escritor) */
        
       
        close(fd[0]);
        
        
        nDadosTx = write(fd[1], mensagem_filho, strlen(mensagem_filho) + 1);
        printf("Filho: %d dados escritos. Conteúdo: \"%s\"\n", nDadosTx, mensagem_filho);
        
        
        close(fd[1]); 
        exit(0);
        
    } else {
        /* PROCESSO PAI (Leitor) */
        
        
        close(fd[1]); 
        
       
        nDadosRx = read(fd[0], buffer_pai, sizeof(buffer_pai));
        printf("Pai: %d dados lidos. Conteúdo: \"%s\"\n", nDadosRx, buffer_pai);
        
        
        close(fd[0]); 
        wait(NULL);
    }

    return 0;
}