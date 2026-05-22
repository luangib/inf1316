#include <stdio.h>
#include <unistd.h>
#include <stdint.h> // Para intptr_t

void *memorypool;

// Salva o limite inicial do heap [cite: 52]
void initmemorypool(void) { 
    memorypool = sbrk(0); 
}

// Aloca n bytes movendo o limite do heap [cite: 47, 48]
void *myallocate(int n) {
    return sbrk(n);
}

// Restaura o heap para o limite inicial salvo 
void resetmemorypool(void){
    brk(memorypool);
}

int main(void) {
    initmemorypool();
    printf("Limite original do heap: %p\n", sbrk(0));

    // Alocando espaço para 100 inteiros
    int *array = (int *)myallocate(100 * sizeof(int));
    printf("Limite após alocar: %p\n", sbrk(0));

    // Usando a memória
    array[0] = 42;
    array[99] = 84;
    printf("array[0] = %d, array[99] = %d\n", array[0], array[99]);

    // Desalocando tudo [cite: 57, 58]
    resetmemorypool();
    printf("Limite após desalocar: %p\n", sbrk(0));

    return 0;
}