#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define NUM_APPS 5

// Definição de Estados do Processo
#define PRONTO 0
#define BLOQUEADO 1

// --- TABELA DE PROCESSOS (PCB) ---
pid_t pids[NUM_APPS];
char *app_names[] = {"A1", "A2", "A3", "A4", "A5"};
int estado[NUM_APPS]; 
int pc_simulado[NUM_APPS]; // Variável para simular o PC (Program Counter)
int acessos_d1[NUM_APPS];  // Contador de acessos ao dispositivo D1

int processo_atual = 0; 
int fd_leitura; 

// --- TRATADOR DE INTERRUPÇÕES ---
void trata_interrupcao(int sig) {
    if (sig == SIGALRM) {
        // --- IRQ0: TIMER ---
        printf("\n[KERNEL] => Recebeu IRQ0 (Timer). Trocando contexto...\n");
        
        if (estado[processo_atual] == PRONTO) {
            kill(pids[processo_atual], SIGSTOP);
            printf("[KERNEL] Pausou %s (PID: %d)\n", app_names[processo_atual], pids[processo_atual]);
        }

        do {
            processo_atual = (processo_atual + 1) % NUM_APPS;
        } while (estado[processo_atual] == BLOQUEADO);

        kill(pids[processo_atual], SIGCONT);
        printf("[KERNEL] Retomou %s (PID: %d)\n\n", app_names[processo_atual], pids[processo_atual]);
        
        // Simula o avanço do PC enquanto o processo ganha a CPU
        pc_simulado[processo_atual] += 150; 
    } 
    else if (sig == SIGUSR1) {
        // --- SYSTEM CALL: PEDIDO DE I/O ---
        char buffer[100];
        read(fd_leitura, buffer, sizeof(buffer));
        printf("\n[KERNEL-SYSCALL] => Recebeu via Pipe: '%s'\n", buffer);
        
        char nome_app[3];
        strncpy(nome_app, buffer, 2);
        nome_app[2] = '\0';

        int idx_bloqueado = -1;
        for(int i = 0; i < NUM_APPS; i++) {
            if(strcmp(app_names[i], nome_app) == 0) {
                idx_bloqueado = i;
                break;
            }
        }

        if (idx_bloqueado != -1) {
            estado[idx_bloqueado] = BLOQUEADO;
            acessos_d1[idx_bloqueado]++; // Incrementa o relatório de uso
            
            printf("[KERNEL] O processo %s mudou para o estado BLOQUEADO (Aguardando D1).\n", app_names[idx_bloqueado]);
            
            if (processo_atual == idx_bloqueado) {
                do {
                    processo_atual = (processo_atual + 1) % NUM_APPS;
                } while (estado[processo_atual] == BLOQUEADO);
                
                kill(pids[processo_atual], SIGCONT);
                printf("[KERNEL] Escalonou imediatamente %s (PID: %d).\n\n", app_names[processo_atual], pids[processo_atual]);
            }
        }
    }
    else if (sig == SIGUSR2) {
        // --- IRQ1: D1 TERMINOU ---
        printf("\n[HARDWARE] =====> IRQ1 RECEBIDA! DISPOSITIVO D1 TERMINOU A LEITURA. <=====\n");
        for(int i = 0; i < NUM_APPS; i++) {
            if (estado[i] == BLOQUEADO) {
                estado[i] = PRONTO;
                printf("[KERNEL] Processo %s teve sua E/S concluida e agora esta PRONTO!\n", app_names[i]);
            }
        }
        printf("\n");
    }
    else if (sig == SIGTSTP) {
        // --- RELATÓRIO DO CTRL+Z ---
        printf("\n\n=========================================================\n");
        printf("||             RELATORIO DO SISTEMA (CTRL+Z)           ||\n");
        printf("=========================================================\n");
        for (int i = 0; i < NUM_APPS; i++) {
            printf("PROCESSO: %s (PID: %d)\n", app_names[i], pids[i]);
            
            // Lógica para saber se está executando agora ou apenas pronto/bloqueado
            if (processo_atual == i && estado[i] == PRONTO) {
                printf("  -> Estado: EXECUTANDO (Na CPU agora!)\n");
            } else if (estado[i] == PRONTO) {
                printf("  -> Estado: PRONTO (Aguardando a CPU)\n");
            } else {
                printf("  -> Estado: BLOQUEADO (Aguardando Dispositivo D1, Operacao de Leitura)\n");
            }
            
            printf("  -> Valor do PC: %d\n", pc_simulado[i]);
            printf("  -> Acessos ao D1: %d vezes\n", acessos_d1[i]);
            printf("---------------------------------------------------------\n");
        }
        printf("=========================================================\n");
        printf("[KERNEL] O simulador imprimiu o relatorio. Para retomar, digite 'fg' no terminal e aperte ENTER.\n\n");
        
        // Pausa todos os filhos antes do Kernel se pausar
        kill(pids[processo_atual], SIGSTOP); 
        
        // Retorna o comportamento padrão do SIGTSTP (pausar o Kernel e jogar pro background)
        signal(SIGTSTP, SIG_DFL);
        raise(SIGTSTP); 
    }
    else if (sig == SIGCONT) {
        // --- RETORNO DO CTRL+Z (Comando 'fg') ---
        // Quando o usuário volta, reativamos o tratador do Ctrl+Z e acordamos o processo atual
        signal(SIGTSTP, trata_interrupcao);
        printf("\n[KERNEL] Simulador retomado! Devolvendo a CPU para %s.\n\n", app_names[processo_atual]);
        kill(pids[processo_atual], SIGCONT);
    }
}

int main() {
    int meu_pid = getpid();
    FILE *arquivo_pid = fopen("kernel.pid", "w");
    if (arquivo_pid != NULL) {
        fprintf(arquivo_pid, "%d", meu_pid);
        fclose(arquivo_pid);
        printf("KernelSim iniciado (PID: %d) - Salvo em kernel.pid\n", meu_pid);
    }

    // Configura os sinais
    signal(SIGALRM, trata_interrupcao); 
    signal(SIGUSR1, trata_interrupcao); 
    signal(SIGUSR2, trata_interrupcao); 
    signal(SIGTSTP, trata_interrupcao); // Escuta o Ctrl+Z
    signal(SIGCONT, trata_interrupcao); // Escuta o 'fg' (Foreground)

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) exit(1);
    fd_leitura = pipe_fd[0];

    for (int i = 0; i < NUM_APPS; i++) {
        estado[i] = PRONTO; 
        pc_simulado[i] = 1000; // Valor inicial do PC fictício
        acessos_d1[i] = 0;

        pid_t pid = fork();
        if (pid == 0) {
            close(pipe_fd[0]); 
            char str_fd[10];
            sprintf(str_fd, "%d", pipe_fd[1]);
            char *args[] = {"./app", app_names[i], str_fd, NULL};
            execvp(args[0], args);
            exit(1);
        } else {
            pids[i] = pid;
        }
    }
    close(pipe_fd[1]);

    for(int i = 1; i < NUM_APPS; i++) {
        kill(pids[i], SIGSTOP);
    }
    printf("\n[KERNEL] Apenas %s iniciou com a CPU.\n\n", app_names[0]);

    while(1) pause(); 
    return 0;
}