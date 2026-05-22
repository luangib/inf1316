#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Verifica se o arquivo foi passado como argumento
    if (argc < 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        exit(1);
    }

    struct stat sb;
    // Abre o arquivo com permissão de leitura e escrita (O_RDWR)
    int fd = open(argv[1], O_RDWR);
    if (fd == -1) { perror("open"); exit(1); }
    if (fstat(fd, &sb) == -1) { perror("fstat"); exit(1); }

    // a. Verificar quantas páginas de memória o arquivo precisou [cite: 167]
    long pagesize = sysconf(_SC_PAGESIZE); 
    long num_pages = (sb.st_size + pagesize - 1) / pagesize; // Divisão com teto
    
    printf("--- INFORMACOES DE MEMORIA ---\n");
    printf("Tamanho do arquivo: %lld bytes\n", (long long)sb.st_size);
    printf("Tamanho da página do sistema: %ld bytes\n", pagesize);
    printf("Total de páginas alocadas: %ld\n\n", num_pages);

    // b. Medir o tempo de escrita: mmap vs E/S em disco [cite: 168-169]
    struct timeval start, end;
    long msec_mmap, msec_write;

    printf("--- TESTE DE PERFORMANCE ---\n");

    // --- Teste 1: Escrita via Mapeamento de Memória (mmap) ---
    char *p = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED) { perror("mmap"); exit(1); }

    gettimeofday(&start, NULL); // Inicia cronômetro
    for (off_t i = 0; i < sb.st_size; i++) {
        p[i] = 'A'; // Escrevendo o caractere 'A' diretamente na RAM
    }
    msync(p, sb.st_size, MS_SYNC); // Sincroniza as alterações da RAM para o disco
    gettimeofday(&end, NULL);   // Para cronômetro
    
    msec_mmap = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    // --- Teste 2: Escrita via I/O tradicional (write) ---
    lseek(fd, 0, SEEK_SET); // Volta o cursor para o começo do arquivo
    char *buffer = malloc(sb.st_size);
    memset(buffer, 'B', sb.st_size); // Preenche o buffer com 'B'

    gettimeofday(&start, NULL); // Inicia cronômetro
    write(fd, buffer, sb.st_size); // Chama a syscall write
    fsync(fd); // Força a gravação física no disco
    gettimeofday(&end, NULL);   // Para cronômetro

    msec_write = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    printf("Tempo de escrita via mmap(): %ld microsegundos\n", msec_mmap);
    printf("Tempo de escrita via write(): %ld microsegundos\n", msec_write);

    // Limpeza e encerramento
    free(buffer);
    munmap(p, sb.st_size);
    close(fd);

    return 0;
}