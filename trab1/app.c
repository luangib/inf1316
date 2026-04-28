#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h> // Necessário para gerar a semente aleatória

// Define o limite de iterações do processo (entre 2000 e 5000, conforme exigido)
#define MAX_ITERATIONS 3000

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Erro: Faltam argumentos.\n");
        exit(1);
    }

    char *nome = argv[1]; 
    int write_fd = atoi(argv[2]); 
    int PC = 0; // Program Counter (contador de instruções)

    // Semente randômica única para CADA processo. 
    // Usamos XOR (^) com getpid() para garantir que A1, A2, etc., não sorteiem os mesmos números.
    srand(time(NULL) ^ getpid());

    // Cada processo de aplicação Ax deve consistir em um loop (de até MAX iterações)
    while(PC < MAX_ITERATIONS) {
        // Acessa uma página de memória virtual randômica entre m00 e m15
        int mem = rand() % 16; 
        
        printf("Processo %s (PID: %d) executando... PC: %d, Mem: m%02d\n", nome, getpid(), PC, mem);
        
        // Sorteio randômico com baixa probabilidade (aprox. 15%) de requisitar uma syscall
        int d = rand() % 100;
        
        if ((d + 1) < 15) { 
            char dispositivo[3];
            char operacao;
            
            // Escolhe randômicamente o dispositivo: D1 ou D2
            if (rand() % 2 == 0) {
                strcpy(dispositivo, "D1");
            } else {
                strcpy(dispositivo, "D2");
            }
            
            // Escolhe randômicamente a operação: R (Read), W (Write) ou X (eXecute)
            int op_rand = rand() % 3;
            if (op_rand == 0) operacao = 'R';
            else if (op_rand == 1) operacao = 'W';
            else operacao = 'X';

            // Monta a mensagem que será enviada pelo Pipe com o CONTEXTO completo
            // Formato: NOME DISPOSITIVO OPERACAO MEMORIA PC (Ex: "A3 D2 W m05 1042")
            char mensagem[50];
            sprintf(mensagem, "%s %s %c m%02d %d", nome, dispositivo, operacao, mem, PC);
            
            // Envia o pedido para o Kernel
            write(write_fd, mensagem, strlen(mensagem) + 1);
            printf("\n>>> %s EXECUTOU SYSCALL: Disp: %s | Op: %c | Mem: m%02d <<<\n", nome, dispositivo, operacao, mem);
            
            // TRAP: Acorda o Kernel avisando que tem Syscall no pipe
            kill(getppid(), SIGUSR1);
            
            // Pausa a si mesmo (simula o Kernel bloqueando a aplicação para I/O)
            kill(getpid(), SIGSTOP); 
        }
        
        PC++;
        usleep(500000); // Pausa de 0.5s para emular o tempo de processamento sem floodar o terminal
    }
    
    printf("\nProcesso %s finalizou suas %d iteracoes com sucesso.\n", nome, MAX_ITERATIONS);
    return 0;
}