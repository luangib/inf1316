#include <stdio.h>       // [cite: 131]
#include <sys/types.h>   // [cite: 132]
#include <sys/stat.h>    // [cite: 133]
#include <fcntl.h>       // [cite: 134]
#include <unistd.h>      // [cite: 135]
#include <sys/mman.h>    // [cite: 136]
#include <stdlib.h>      // Necessário para o exit()

int main (int argc, char *argv[]) // [cite: 125]
{ 
    struct stat sb;      // [cite: 127]
    int fd;              // [cite: 128]
    off_t len;           // [cite: 129]
    char *p;             // [cite: 130]

    // Verifica se o usuário passou o nome do arquivo como argumento
    if (argc < 2) { 
        fprintf (stderr, "usage: %s <file>\n", argv[0]); // [cite: 138-139]
        exit(1);                                         // [cite: 140]
    } 

    // Abre o arquivo apenas para leitura
    fd = open (argv[1], O_RDONLY);                       // [cite: 141]
    if (fd == -1) { perror ("open"); exit(1); }          // [cite: 142]

    // Pega as informações do arquivo (como o tamanho st_size)
    if (fstat(fd, &sb) == -1) { perror ("fstat"); exit(1); } // [cite: 143-144]

    // Verifica se é um arquivo regular
    if (!S_ISREG (sb.st_mode)) { 
        fprintf (stderr, "%s is not a file\n", argv[1]); // [cite: 146-147]
        exit(1);                                         // [cite: 147]
    }

    // Faz o mapeamento do arquivo inteiro para a memória
    p = mmap (0, sb.st_size, PROT_READ, MAP_SHARED, fd, 0); // [cite: 148-149]
    if (p == MAP_FAILED) { perror ("mmap"); exit(1); }      // [cite: 148]

    // O descritor pode ser fechado sem invalidar o mapeamento
    if (close (fd) == -1) { perror ("close"); exit(1); }    // 

    // Imprime o arquivo lendo diretamente da memória (como se fosse um array)
    for (len = 0; len < sb.st_size; len++) putchar (p[len]); // 

    // Desfaz o mapeamento
    if (munmap (p, sb.st_size) == -1) { perror ("munmap"); exit(1); } // [cite: 153]
    
    return 0; // [cite: 153]
} // [cite: 154]