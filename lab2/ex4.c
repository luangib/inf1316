//ex4.c


#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

int main() {
    // 1. Cria as memórias com os números diretos
    int id1 = shmget(7000, sizeof(int) * 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    int id2 = shmget(8000, sizeof(int) * 2, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    int *m1 = (int *) shmat(id1, 0, 0);
    int *m2 = (int *) shmat(id2, 0, 0);

    m1[1] = 0; m2[1] = 0;

    // 2. Chama os dois programas diferentes
    if (fork() == 0) {
        execl("./p1", "p1", NULL); // Roda o executável p1
        exit(0);
    }
    if (fork() == 0) {
        execl("./p2", "p2", NULL); // Roda o executável p2
        exit(0);
    }

    printf("(Pai // pid %d) Esperando P1 e P2...\n", getpid());
    while(m1[1] == 0 || m2[1] == 0) {
        usleep(100000); // usei pq é mais preciso que o sleep normal(tem como colocar menor que 1s)
    }

    printf("\n(Pai // pid %d) Resultado: %d * %d = %d\n",getpid(), m1[0], m2[0], m1[0] * m2[0]);

    // Limpeza
    shmdt(m1); shmdt(m2);
    shmctl(id1, IPC_RMID, NULL);
    shmctl(id2, IPC_RMID, NULL);

    return 0;
}