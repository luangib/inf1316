#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void trata_fpe(int sinal) {
    printf("\n[SINAL] Recebi um SIGFPE\n");
    exit(1);
}

int main() {
    int n1, n2; 

    signal(SIGFPE, trata_fpe);

    printf("Digite o 1º número (inteiro): ");
    scanf("%d", &n1);
    printf("Digite o 2º número (inteiro 0): ");
    scanf("%d", &n2);

    printf("\n--- Resultados ---\n");
    printf("Soma: %d\n", n1 + n2);
    printf("Subtração: %d\n", n1 - n2);
    printf("Multiplicação: %d\n", n1 * n2);
    printf("Divisão: %d\n", n1 / n2); 

    return 0;
}