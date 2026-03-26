//filho_rapido.c

#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Filho: Começando execução rápida...\n");
    sleep(2); 
    printf("Filho: Terminei meu trabalho!\n");
    return 0;
}