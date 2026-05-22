#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    const char *filepath = "texto_ex5.txt";
    
    // 1. Cria o arquivo inicial
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) { perror("open"); exit(1); }

    // Define um tamanho inicial de 15 bytes e mapeia
    size_t tamanho_antigo = 15;
    ftruncate(fd, tamanho_antigo); 
    
    char *p = mmap(0, tamanho_antigo, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); exit(1); }

    strcpy(p, "Texto base. "); // Escreve o conteúdo inicial
    printf("Conteúdo inicial na RAM: %s\n", p);

    // 2. A SOLUÇÃO: Para não tomar SegFault ao escrever no final do arquivo,
    // precisamos expandir o tamanho dele no disco ANTES de escrever.
    size_t tamanho_novo = 45;
    ftruncate(fd, tamanho_novo);

    // 3. Desfaz o mapeamento antigo
    munmap(p, tamanho_antigo);

    // 4. Faz um NOVO mapeamento com o tamanho expandido
    p = mmap(0, tamanho_novo, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap novo"); exit(1); }
    
    // 5. Agora é totalmente seguro adicionar dados no final do arquivo!
    strcat(p, "Adicionando novos caracteres!"); // [cite: 173]
    printf("Conteúdo após expansão: %s\n", p);

    // Limpeza
    munmap(p, tamanho_novo);
    close(fd);
    
    return 0;
}