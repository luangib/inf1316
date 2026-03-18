//p1.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <unistd.h>
#include <time.h>

int main() {
    // Usando o número 7000 direto no comando
    int id = shmget(7000, sizeof(int) * 2, 0);
    int *m1 = (int *) shmat(id, 0, 0);

    srand(getpid()); // fiz isso para não dar sempre o mesmo nos dois(só fiz no p1)
    sleep(rand() % 5 + 1); //Limitei pq tava demorando demais as vezes

    m1[0] = rand() % 10 + 1; 
    m1[1] = 1;              

    printf("(P1 pid %d) Valor %d enviado na chave 7000.\n",getpid(), m1[0]);
    shmdt(m1);
    return 0;
}