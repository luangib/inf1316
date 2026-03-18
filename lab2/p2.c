//p2.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

int main() {
    // Usando o número 8000 fixo 
    int id = shmget(8000, sizeof(int) * 2, 0);
    int *m2 = (int *) shmat(id, 0, 0);

    sleep(rand() % 5 + 1); //Limitei pq tava demorando demais as vezes

    m2[0] = rand() % 10 + 1;
    m2[1] = 1;              

    printf("(P2 // pid %d) Valor %d enviado na chave 8000.\n",getpid(), m2[0]);
    shmdt(m2);
    return 0;
}