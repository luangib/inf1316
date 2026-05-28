#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_KEY 999
#define MIN_KEY 0

typedef enum {
    READ, WRITE
} OpType;

// Função original dos slides que gera e exibe N operações aleatórias [cite: 244, 245]
void gerar_operacoes(int n) {
    // Inicializa o gerador de números aleatórios com o tempo atual [cite: 246]
    srand(time(NULL));
    
    for (int i = 0; i < n; i++) {
        // Escolhe aleatoriamente entre READ (0) e WRITE (1) [cite: 248, 249]
        OpType op = rand() % 2;
        
        // Gera key aleatória no intervalo de 0 a 999 [cite: 250, 251]
        int key = (rand() % (MAX_KEY - MIN_KEY + 1)) + MIN_KEY;
        
        if (op == READ) {
            // Formato pedido: R, key [cite: 253, 254]
            printf("R, %d\n", key);
        } else {
            // Gera um valor inteiro aleatório qualquer (ex: entre 10 e 5000) [cite: 256, 257]
            int value = (rand() % 4991) + 10;
            // Formato pedido: W, value, key [cite: 258, 259]
            printf("W, %d, %d\n", value, key);
        }
    }
}

int main() {
    // Redireciona a saída do printf para o arquivo log.txt
    // "w" abre o arquivo para escrita, sobrescrevendo se já existir
    if (freopen("log.txt", "w", stdout) == NULL) {
        fprintf(stderr, "Erro ao abrir (ou criar) o arquivo log.txt\n");
        return 1;
    }

    // Chama a função para gerar as 10.000 operações solicitadas no exercício [cite: 314]
    gerar_operacoes(10000);

    return 0;
}