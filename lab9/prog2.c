#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main() {
    const char *filepath = "memoria_compartilhada.bin";
    size_t tamanho = 4096;

    // Abre o arquivo já existente (apenas leitura)
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) { 
        perror("open prog2 (Rode o prog1 primeiro!)"); 
        exit(1); 
    }

    // Mapeia usando MAP_SHARED para ver as atualizações em tempo real 
    char *p = mmap(0, tamanho, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap prog2"); exit(1); }

    // Lê os dados como se fossem uma string normal
    printf("[Prog2] Lendo a memória compartilhada:\n");
    printf("--> \"%s\"\n", p);

    // Limpeza
    munmap(p, tamanho);
    close(fd);
    
    return 0;
}