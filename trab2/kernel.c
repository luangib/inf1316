#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#define NUM_APPS 5
#define PRONTO 0
#define BLOQUEADO 1

// --- DEFINICOES PARA MEMORIA VIRTUAL ---
#define NUM_FRAMES 32
#define NUM_PAGES 16

typedef struct {
    int valid;      
    int frame;      
    int modifyBit;  
    int when;       
} PageTableEntry;

typedef struct {
    pid_t pid;
    char nome[3];
    int estado;
    int pc;
    char mem_atual[5];
    char disp_bloqueio[3]; 
    char op_bloqueio;      
    int acessos_d1;
    int acessos_d2;
    
    PageTableEntry TP[NUM_PAGES]; 
    int total_page_faults;
    int total_page_faults_duplos;
} PCB;

PCB tabela[NUM_APPS];
int processo_atual = 0;
int fd_leitura;

// Arrays Globais de Memoria (RAM)
int RAM[NUM_FRAMES];       
int RAM_free[NUM_FRAMES];  

// Filas de Dispositivo FIFO
int fila_d1[NUM_APPS], frente_d1 = 0, tras_d1 = 0;
int fila_d2[NUM_APPS], frente_d2 = 0, tras_d2 = 0;

// NOVO: Fila de Swap (Sw_Queue)
typedef struct {
    int pid_idx;      // Qual processo
    int mem_req;      // Qual pagina ele quer
    int espera;       // Quantos IRQ3 ele precisa esperar (1 ou 2)
} SwapRequest;

SwapRequest fila_swap[100]; // Array maior para permitir re-enfileiramento
int frente_swap = 0, tras_swap = 0;

void enfileirar_swap(int idx, int pagina, int espera) {
    fila_swap[tras_swap].pid_idx = idx;
    fila_swap[tras_swap].mem_req = pagina;
    fila_swap[tras_swap].espera = espera;
    tras_swap++;
}

void enfileirar(int *fila, int *tras, int idx) { fila[(*tras)++] = idx; }
int desenfileirar(int *fila, int *frente, int *tras) {
    if (*frente == *tras) return -1;
    return fila[(*frente)++];
}

// --- FASE 5: ALGORITMOS DE SUBSTITUICAO ---

// Substituicao Global: Procura o quadro mais antigo na RAM inteira
int global_substitute() {
    int oldest_frame = 0;
    int min_when = 9999999; // Começa com um valor bem alto
    
    for (int f = 0; f < NUM_FRAMES; f++) {
        if (RAM_free[f] == 0) { // Se o quadro está ocupado
            int proc_v = RAM[f] / 100; // Descobre o dono
            int pag_v = RAM[f] % 100;  // Descobre a pagina
            
            // Verifica se o tempo de acesso dessa pagina eh o menor ate agora
            if (tabela[proc_v].TP[pag_v].when < min_when) {
                min_when = tabela[proc_v].TP[pag_v].when;
                oldest_frame = f;
            }
        }
    }
    return oldest_frame;
}

// Substituicao Local: Procura o quadro mais antigo pertencente ao proprio processo
int local_substitute(int proc_idx) {
    int oldest_frame = -1;
    int min_when = 9999999;
    
    for (int f = 0; f < NUM_FRAMES; f++) {
        // So olha para os quadros se o dono for igual ao proc_idx
        if (RAM_free[f] == 0 && (RAM[f] / 100) == proc_idx) {
            int pag_v = RAM[f] % 100;
            
            if (tabela[proc_idx].TP[pag_v].when < min_when) {
                min_when = tabela[proc_idx].TP[pag_v].when;
                oldest_frame = f;
            }
        }
    }
    
    // Tratamento de Excecao: E se a RAM esta cheia, mas ESSE processo ainda 
    // nao tem nenhuma pagina la para substituir? Faremos um fallback para a Global.
    if (oldest_frame == -1) {
        return global_substitute();
    }
    
    return oldest_frame;
}

// Tratador de Interrupcoes
void trata_interrupcao(int sig) {
    if (sig == SIGALRM) { 
        printf("\n[KERNEL] => IRQ0 (Timer). Escalonamento Round-Robin.\n");
        
        if (tabela[processo_atual].estado == PRONTO) {
            kill(tabela[processo_atual].pid, SIGSTOP);
        }

        int todos_bloqueados = 1;
        for(int i = 0; i < NUM_APPS; i++) {
            if(tabela[i].estado == PRONTO) { todos_bloqueados = 0; break; }
        }

        if (todos_bloqueados) {
            printf("[KERNEL] CPU Ociosa. Todos os processos estao bloqueados aguardando hardware...\n");
        } else {
            do {
                processo_atual = (processo_atual + 1) % NUM_APPS;
            } while (tabela[processo_atual].estado == BLOQUEADO);

            kill(tabela[processo_atual].pid, SIGCONT);
            printf("[KERNEL] CPU atribuida a %s (PID: %d)\n", tabela[processo_atual].nome, tabela[processo_atual].pid);
        }
    } 
    else if (sig == SIGUSR1) { 
        char buffer[100];
        read(fd_leitura, buffer, sizeof(buffer));
        
        int idx = processo_atual;
        sscanf(buffer, "%*s %s %c %s %d", tabela[idx].disp_bloqueio, &tabela[idx].op_bloqueio, tabela[idx].mem_atual, &tabela[idx].pc);
        
        // NOVO: Fase 4 - Logica de Interceptacao de Memoria Virtual
        if (strcmp(tabela[idx].disp_bloqueio, "M") == 0) {
            int pagina_solicitada = atoi(&tabela[idx].mem_atual[1]); // Extrai o numero (ex: "m05" -> 5)
            
            if (tabela[idx].TP[pagina_solicitada].valid == 1) {
                // PAGE HIT: A pagina ja esta na RAM
                printf("[KERNEL] PAGE HIT! Processo %s acessou direto a pagina m%02d na RAM.\n", tabela[idx].nome, pagina_solicitada);
                tabela[idx].TP[pagina_solicitada].when = tabela[idx].pc; // Atualiza o relogio de acesso
                if (tabela[idx].op_bloqueio == 'W') {
                    tabela[idx].TP[pagina_solicitada].modifyBit = 1; // Sujou a pagina
                }
                // Como foi direto na RAM, nao bloqueia, continua PRONTO
                tabela[idx].estado = PRONTO; 
            } else {
                // PAGE FAULT: Pagina nao esta na RAM
                tabela[idx].total_page_faults++;
                tabela[idx].estado = BLOQUEADO;
                printf("[KERNEL] PAGE FAULT! Processo %s bloqueado aguardando pagina m%02d do Swap.\n", tabela[idx].nome, pagina_solicitada);
                
                // Procura um quadro livre na RAM
                int frame_livre = -1;
                for(int f = 0; f < NUM_FRAMES; f++) {
                    if (RAM_free[f] == 1) { frame_livre = f; break; }
                }

                int espera_necessaria;
                if (frame_livre != -1) {
                    // Achou espaco vazio. Preenche e avisa que precisa de 1 IRQ3
                    RAM_free[frame_livre] = 0;
                    RAM[frame_livre] = (idx * 100) + pagina_solicitada; // Assinatura simples de quem ocupa
                    tabela[idx].TP[pagina_solicitada].frame = frame_livre;
                    espera_necessaria = 1;
                } else {
                    // --- FASE 5: RAM CHEIA! ALGORITMO DE SUBSTITUICAO ---
                    
                    // ESCOLHA AQUI O SEU ALGORITMO (comente um e descomente o outro para testar):
                    int frame_vitima = global_substitute(); 
                    // int frame_vitima = local_substitute(idx);
                    
                    // 1. Descobrimos quem eh a vitima que vai ser removida
                    int proc_vitima = RAM[frame_vitima] / 100;
                    int pag_vitima = RAM[frame_vitima] % 100;
                    
                    printf("[KERNEL] ALERTA DE DESPEJO! Processo %s vai roubar o quadro %d do processo %s (Pagina m%02d).\n", 
                           tabela[idx].nome, frame_vitima, tabela[proc_vitima].nome, pag_vitima);
                    
                    // 2. Verificamos se o caderno antigo estava sujo (modifyBit == 1)
                    if (tabela[proc_vitima].TP[pag_vitima].modifyBit == 1) {
                        espera_necessaria = 2; // Precisa salvar no swap primeiro
                        tabela[idx].total_page_faults_duplos++;
                    } else {
                        espera_necessaria = 1; // Pode descartar direto
                    }
                    
                    // 3. Atualiza a Tabela de Paginas da vitima (A pagina nao esta mais na RAM)
                    tabela[proc_vitima].TP[pag_vitima].valid = 0;
                    tabela[proc_vitima].TP[pag_vitima].frame = -1;
                    
                    // 4. O novo processo toma posse do quadro da RAM
                    RAM[frame_vitima] = (idx * 100) + pagina_solicitada;
                    tabela[idx].TP[pagina_solicitada].frame = frame_vitima;
                }

                enfileirar_swap(idx, pagina_solicitada, espera_necessaria);
            }
        } 
        // Logica Antiga para D1 e D2
        else {
            tabela[idx].estado = BLOQUEADO;
            printf("[KERNEL] Syscall: %s bloqueado por %s (%c). PC: %d\n", 
                    tabela[idx].nome, tabela[idx].disp_bloqueio, tabela[idx].op_bloqueio, tabela[idx].pc);

            if (strcmp(tabela[idx].disp_bloqueio, "D1") == 0) {
                enfileirar(fila_d1, &tras_d1, idx);
                tabela[idx].acessos_d1++;
            } else {
                enfileirar(fila_d2, &tras_d2, idx);
                tabela[idx].acessos_d2++;
            }
        }

        // Checa se precisa passar a CPU adiante
        int todos_bloqueados = 1;
        for(int i = 0; i < NUM_APPS; i++) {
            if(tabela[i].estado == PRONTO) { todos_bloqueados = 0; break; }
        }

        if (todos_bloqueados) {
            printf("[KERNEL] CPU Ociosa...\n");
        } else if (tabela[processo_atual].estado == BLOQUEADO) {
            do { processo_atual = (processo_atual + 1) % NUM_APPS; } while (tabela[processo_atual].estado == BLOQUEADO);
            kill(tabela[processo_atual].pid, SIGCONT);
        } else {
            // Se foi um Page Hit, o processo_atual continua rodando
            kill(tabela[processo_atual].pid, SIGCONT);
        }
    }
    // NOVO: Tratamento do Retorno do Disco de Swap (IRQ3)
    else if (sig == SIGIO) {
        if (frente_swap != tras_swap) {
            SwapRequest req = fila_swap[frente_swap];
            frente_swap++; // Retira da frente

            req.espera--; // Decrementa a espera

            if (req.espera == 0) {
                // Terminou! A pagina esta na RAM.
                int idx = req.pid_idx;
                tabela[idx].estado = PRONTO;
                tabela[idx].TP[req.mem_req].valid = 1;
                tabela[idx].TP[req.mem_req].when = tabela[idx].pc;
                
                if (tabela[idx].op_bloqueio == 'W') {
                    tabela[idx].TP[req.mem_req].modifyBit = 1;
                } else {
                    tabela[idx].TP[req.mem_req].modifyBit = 0;
                }
                printf("\n[HARDWARE] =====> IRQ3: Swap Concluido! Processo %s tem a pagina m%02d na RAM <=====\n", tabela[idx].nome, req.mem_req);
            } else {
                // Ainda precisa de mais um IRQ3 (salvando pagina suja anterior)
                printf("\n[HARDWARE] =====> IRQ3: Salvamento parcial. %s precisa de mais um ciclo <=====\n", tabela[req.pid_idx].nome);
                enfileirar_swap(req.pid_idx, req.mem_req, req.espera); // Volta pro final da fila
            }
        }
    }
    else if (sig == SIGUSR2 || sig == SIGURG) { 
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
    else if (sig == SIGTSTP) { 
        printf("\n\n==================== RELATORIO DO SISTEMA ====================\n");
        for (int i = 0; i < NUM_APPS; i++) {
            printf("APP: %s | PID: %d | PC: %d | Mem: %s\n", tabela[i].nome, tabela[i].pid, tabela[i].pc, tabela[i].mem_atual);
            if (tabela[i].estado == BLOQUEADO)
                printf("  -> Estado: BLOQUEADO (%s - Op: %c)\n", tabela[i].disp_bloqueio, tabela[i].op_bloqueio);
            else
                printf("  -> Estado: %s\n", (processo_atual == i && tabela[i].estado != BLOQUEADO) ? "EXECUTANDO" : "PRONTO");
            printf("  -> Total Acessos I/O: D1: %d | D2: %d\n", tabela[i].acessos_d1, tabela[i].acessos_d2);
            printf("  -> Page Faults: Totais: %d | Duplos: %d\n", tabela[i].total_page_faults, tabela[i].total_page_faults_duplos);
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

    for(int i = 0; i < NUM_FRAMES; i++) {
        RAM_free[i] = 1; 
        RAM[i] = -1;     
    }

    signal(SIGALRM, trata_interrupcao); 
    signal(SIGUSR1, trata_interrupcao);
    signal(SIGUSR2, trata_interrupcao); 
    signal(SIGURG, trata_interrupcao);
    signal(SIGIO, trata_interrupcao); // NOVO: Escutando o IRQ3
    signal(SIGTSTP, trata_interrupcao); 
    signal(SIGCONT, trata_interrupcao);

    int pipe_fd[2]; pipe(pipe_fd);
    fd_leitura = pipe_fd[0];

    for (int i = 0; i < NUM_APPS; i++) {
        sprintf(tabela[i].nome, "A%d", i + 1);
        tabela[i].estado = PRONTO; tabela[i].pc = 0;
        tabela[i].acessos_d1 = 0; tabela[i].acessos_d2 = 0;
        strcpy(tabela[i].mem_atual, "m00");
        
        tabela[i].total_page_faults = 0;
        tabela[i].total_page_faults_duplos = 0;
        for(int j = 0; j < NUM_PAGES; j++) {
            tabela[i].TP[j].valid = 0;      
            tabela[i].TP[j].frame = -1;     
            tabela[i].TP[j].modifyBit = 0;  
            tabela[i].TP[j].when = 0;       
        }

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