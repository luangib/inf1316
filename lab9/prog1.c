#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main() {
    const char *filepath = "memoria_compartilhada.bin";
    size_t tamanho = 4096; // Tamanho de 1 página do sistema

    // Cria/abre o arquivo para leitura e escrita
    int fd = open(filepath, O_RDWR | O_CREAT, 0666);
    if (fd == -1) { perror("open prog1"); exit(1); }

    // Define o tamanho real do arquivo no disco
    ftruncate(fd, tamanho);

    // Mapeia a memória com MAP_SHARED 
    char *p = mmap(0, tamanho, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap prog1"); exit(1); }

    // Escreve os dados diretamente na memória mapeada
    printf("[Prog1] Escrevendo mensagem na memória...\n");
    strcpy(p, "Salve do Prog1! Essa mensagem não passou por pipes nem sockets.");

    // Pausa para dar tempo de rodar o Prog2
    printf("[Prog1] Mensagem escrita. Deixando em aberto por 15 segundos...\n");
    sleep(15); 

    // Limpeza
    munmap(p, tamanho);
    close(fd);
    printf("[Prog1] Encerrado.\n");
    
    return 0;
}