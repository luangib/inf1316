// questao04.c
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int pid, status;

    // Parte A: Executando programa próprio
    pid = fork();
    if (pid == 0) { // Filho
        printf("[Filho] Troca código pelo 'mensagem'\n");
        execl("./mensagem", "mensagem", NULL);
        
        perror("Erro ao executar execl");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
        printf("[Pai]'mensagem' terminou.\n\n");
    }

    // Parte B: Executando o echo da shell
    pid = fork();
    if (pid == 0) {
        printf("[Filho] Troca ocódigo pelo 'echo' \n");
        execl("/bin/echo", "echo", "Echo: Ola mundo ", NULL);
        
        perror("Erro ao executar execl do echo");
        exit(1);
    } else {
        waitpid(pid, &status, 0);
        printf("[Pai] 'echo' terminou\n");
    }

    return 0;
}