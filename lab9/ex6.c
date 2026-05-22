#define _GNU_SOURCE // Necessário para habilitar o mremap no GCC
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    const char *filepath = "texto_ex6.txt";
    
    // 1. Cria o arquivo inicial
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) { perror("open"); exit(1); }

    // Define um tamanho inicial e mapeia
    size_t old_sz = 15;
    ftruncate(fd, old_sz); 
    
    char *p = mmap(NULL, old_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); exit(1); }

    strcpy(p, "Texto base 6. ");
    printf("Conteúdo inicial na RAM: %s\n", p);

    // 2. Expande o arquivo físico no disco PRIMEIRO (Evita SegFault/Bus Error)
    size_t new_sz = 55;
    ftruncate(fd, new_sz);

    // 3. Redimensiona o mapeamento de memória usando mremap() 
    // A flag MREMAP_MAYMOVE diz ao kernel que ele pode mudar o endereço base
    // caso não haja espaço contíguo suficiente no endereço virtual atual.
    p = mremap(p, old_sz, new_sz, MREMAP_MAYMOVE); 
    if (p == MAP_FAILED) { perror("mremap"); exit(1); }
    
    // 4. Escreve no final do arquivo com segurança
    strcat(p, "Usando mremap para expandir a memoria!");
    printf("Conteúdo após mremap: %s\n", p);

    // Limpeza
    munmap(p, new_sz);
    close(fd);
    
    return 0;
}