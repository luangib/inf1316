#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define NUM_APPS 5
#define PRONTO 0
#define BLOQUEADO 1

// --- ESTRUTURA DE CONTROLO DE PROCESSO (PCB) ---
typedef struct {
    pid_t pid;
    char nome[3];
    int estado;
    int pc;
    char mem_atual[5];
    char disp_bloqueio[3]; // D1 ou D2
    char op_bloqueio;      // R, W ou X
    int acessos_d1;
    int acessos_d2;
} PCB;

PCB tabela[NUM_APPS];
int processo_atual = 0;
int fd_leitura;

// --- FILAS DE DISPOSITIVOS (FIFO) ---
int fila_d1[NUM_APPS], frente_d1 = 0, tras_d1 = 0;
int fila_d2[NUM_APPS], frente_d2 = 0, tras_d2 = 0;

void enfileirar(int *fila, int *tras, int idx) { fila[(*tras)++] = idx; }
int desenfileirar(int *fila, int *frente, int *tras) {
    if (*frente == *tras) return -1;
    return fila[(*frente)++];
}

// --- TRATADOR DE INTERRUPÇÕES ---
void trata_interrupcao(int sig) {
    if (sig == SIGALRM) { // IRQ0: Timer
        printf("\n[KERNEL] => IRQ0 (Timer). Escalonamento Round-Robin.\n");
        
        if (tabela[processo_atual].estado == PRONTO) {
            kill(tabela[processo_atual].pid, SIGSTOP);
        }

        do {
            processo_atual = (processo_atual + 1) % NUM_APPS;
        } while (tabela[processo_atual].estado == BLOQUEADO);

        kill(tabela[processo_atual].pid, SIGCONT);
        printf("[KERNEL] CPU atribuida a %s (PID: %d)\n", tabela[processo_atual].nome, tabela[processo_atual].pid);
    } 
    else if (sig == SIGUSR1) { // SYSCALL: Pedido de I/O
        char buffer[100];
        read(fd_leitura, buffer, sizeof(buffer));
        
        int idx = processo_atual;
        // Parsing do contexto: "A3 D2 W m05 1042"
        sscanf(buffer, "%*s %s %c %s %d", tabela[idx].disp_bloqueio, &tabela[idx].op_bloqueio, tabela[idx].mem_atual, &tabela[idx].pc);
        
        tabela[idx].estado = BLOQUEADO;
        printf("[KERNEL] Syscall: %s bloqueado por %s (%c) na memoria %s. PC: %d\n", 
                tabela[idx].nome, tabela[idx].disp_bloqueio, tabela[idx].op_bloqueio, tabela[idx].mem_atual, tabela[idx].pc);

        if (strcmp(tabela[idx].disp_bloqueio, "D1") == 0) {
            enfileirar(fila_d1, &tras_d1, idx);
            tabela[idx].acessos_d1++;
        } else {
            enfileirar(fila_d2, &tras_d2, idx);
            tabela[idx].acessos_d2++;
        }

        // Escolhe o próximo pronto
        do { processo_atual = (processo_atual + 1) % NUM_APPS; } while (tabela[processo_atual].estado == BLOQUEADO);
        kill(tabela[processo_atual].pid, SIGCONT);
    }
    else if (sig == SIGUSR2 || sig == SIGURG) { // IRQ1 (D1) ou IRQ2 (D2)
        int idx_liberado;
        if (sig == SIGUSR2) {
            printf("\n[HARDWARE] =====> IRQ1: D1 Concluido <=====\n");
            idx_liberado = desenfileirar(fila_d1, &frente_d1, &tras_d1);
        } else {
            printf("\n[HARDWARE] =====> IRQ2: D2 Concluido <=====\n");
            idx_liberado = desenfileirar(fila_d2, &frente_d2, &tras_d2);
        }

        if (idx_liberado != -1) {
            tabela[idx_liberado].estado = PRONTO;
            printf("[KERNEL] Processo %s agora esta PRONTO!\n", tabela[idx_liberado].nome);
        }
    }
    else if (sig == SIGTSTP) { // RELATÓRIO (Ctrl+Z)
        printf("\n\n==================== RELATORIO DO SISTEMA ====================\n");
        for (int i = 0; i < NUM_APPS; i++) {
            printf("APP: %s | PID: %d | PC: %d | Mem: %s\n", tabela[i].nome, tabela[i].pid, tabela[i].pc, tabela[i].mem_atual);
            if (tabela[i].estado == BLOQUEADO)
                printf("  -> Estado: BLOQUEADO (%s - Op: %c)\n", tabela[i].disp_bloqueio, tabela[i].op_bloqueio);
            else
                printf("  -> Estado: %s\n", (processo_atual == i) ? "EXECUTANDO" : "PRONTO");
            printf("  -> Total Acessos: D1: %d | D2: %d\n", tabela[i].acessos_d1, tabela[i].acessos_d2);
            printf("--------------------------------------------------------------\n");
        }
        kill(tabela[processo_atual].pid, SIGSTOP);
        signal(SIGTSTP, SIG_DFL);
        raise(SIGTSTP); 
    }
    else if (sig == SIGCONT) {
        signal(SIGTSTP, trata_interrupcao);
        kill(tabela[processo_atual].pid, SIGCONT);
    }
}

int main() {
    FILE *f = fopen("kernel.pid", "w");
    fprintf(f, "%d", getpid());
    fclose(f);

    signal(SIGALRM, trata_interrupcao); signal(SIGUSR1, trata_interrupcao);
    signal(SIGUSR2, trata_interrupcao); signal(SIGURG, trata_interrupcao);
    signal(SIGTSTP, trata_interrupcao); signal(SIGCONT, trata_interrupcao);

    int pipe_fd[2]; pipe(pipe_fd);
    fd_leitura = pipe_fd[0];

    for (int i = 0; i < NUM_APPS; i++) {
        sprintf(tabela[i].nome, "A%d", i + 1);
        tabela[i].estado = PRONTO; tabela[i].pc = 0;
        tabela[i].acessos_d1 = 0; tabela[i].acessos_d2 = 0;
        strcpy(tabela[i].mem_atual, "m00");

        pid_t pid = fork();
        if (pid == 0) {
            char s_fd[10]; sprintf(s_fd, "%d", pipe_fd[1]);
            execlp("./app", "./app", tabela[i].nome, s_fd, NULL);
        }
        tabela[i].pid = pid;
        if (i > 0) kill(pid, SIGSTOP);
    }
    while(1) pause();
    return 0;
}