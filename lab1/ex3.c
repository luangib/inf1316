#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int pid, status;
    int v[10] = {1, 3, 2, 4, 5, 6, 7, 9, 8, 10};

    printf("PID Principal: %d\n", getpid());
    printf("Vetor original (antes do fork): ");
    for (int i = 0; i < 10; i++) printf("%d ", v[i]);
    printf("\n---\n");

    pid = fork();

    if (pid < 0) {
        return 1;
    }

    if (pid != 0) { // Processo Pai
        waitpid(pid, &status, 0); 

        printf("Pai (apos waitpid): ");
        for (int i = 0; i < 10; i++) printf("%d ", v[i]);
        printf("\n");
    
    } 
    else { // Processo Filho
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9 - i; j++) {
                if (v[j] > v[j + 1]) {
                    int t = v[j];
                    v[j] = v[j + 1];
                    v[j + 1] = t;
                }
            }
        }
        
        printf("Filho (vetor ordenado na copia dele): ");
        for (int i = 0; i < 10; i++) printf("%d ", v[i]);
        printf("\n");
        
        exit(0);
    }

    return 0;
}