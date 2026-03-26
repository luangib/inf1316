//ex3.c

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void childhandler(int signo);
int delay;

int main (int argc, char *argv[])
{
    pid_t pid;

    // Verificação: ./ex3 <tempo_limite> <tipo_filho>
    // tipo_filho deve ser 'r' para rapido ou 'l' para lento
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <tempo_limite> <r/l>\n", argv[0]);
        fprintf(stderr, "Ex: ./ex3 10 r  (para rodar o filho_rapido)\n");
        exit(-1);
    }

    sscanf(argv[1], "%d", &delay); 
    signal(SIGCHLD, childhandler);

    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "Erro ao criar filho\n");
        exit(-1);
    }

    if (pid == 0) /* FILHO */
    {
        // Definindo os caminhos fixos e argumentos para o execve
        char *caminho;
        if (argv[2][0] == 'r') {
            caminho = "./filho_rapido";
        } else {
            caminho = "./filho_lento";
        }

        char *args[] = {caminho, NULL}; // O primeiro argumento é o próprio nome
        char *env[] = {NULL};           // Ambiente vazio

        printf("[FILHO] Vou executar o programa: %s\n", caminho);
        
        // Chamada oficial do execve
        execve(caminho, args, env);

        // Se o arquivo não existir ou não tiver permissão, o código abaixo roda:
        perror("Erro ao executar o execve (verifique se compilou os filhos)");
        exit(-1);
    }
    else /* PAI */
    {
        printf("[PAI] Monitorando por %d segundos...\n", delay);
        sleep(delay);

        // Se o pai acordar e o handler não tiver encerrado, mata o filho
        printf("\n[PAI] Tempo esgotado! Matando o processo filho %d...\n", pid);
        kill(pid, SIGKILL);
        
        sleep(1); 
    }

    return 0;
}

void childhandler(int signo) 
{
    int status;
    pid_t pid = wait(&status);
    
    if (WIFEXITED(status)) {
        printf("\n[HANDLER] Filho %d terminou normalmente (Status: %d).\n", pid, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("\n[HANDLER] Filho %d foi encerrado pelo sinal %d (FILHOCIDIO).\n", pid, WTERMSIG(status));
    }
    
    exit(0); 
}