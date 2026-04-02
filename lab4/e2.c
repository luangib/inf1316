#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h> 

int main() {
    int fd_in, fd_out;
    char buffer[1024];
    ssize_t bytes_lidos;

    fd_in = open("entrada.txt", O_RDONLY);
    if (fd_in < 0) {
        perror("Erro ao abrir entrada.txt (crie o arquivo antes de rodar)");
        exit(1);
    }

    
    fd_out = open("saida.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd_out < 0) {
        perror("Erro ao abrir saida.txt");
        close(fd_in);
        exit(1);
    }

    //Redireciona a entrada padrão
    if (dup2(fd_in, 0) == -1) {
        perror("Erro no dup2 para stdin");
        exit(1);
    }

    //Redireciona a saída padrão
    if (dup2(fd_out, 1) == -1) {
        perror("Erro no dup2 para stdout");
        exit(1);
    }

  
    close(fd_in);
    close(fd_out);

    //Loop de leitura e escrita
    while ((bytes_lidos = read(0, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, bytes_lidos);
    }

    if (bytes_lidos < 0) {
        perror("Erro durante a leitura");
        exit(1);
    }

    return 0;
}