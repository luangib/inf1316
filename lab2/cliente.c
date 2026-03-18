//Cliente.c

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>


int main (int argc, char *argv[])
{

//minhas variaveis 
int segmento, *p, id, pid, status;
int n;

//processo da memoria compartilhada
segmento = shmget (8752, sizeof (int),0);
p = (int *) shmat (segmento, 0, 0); // comparar o retorno com -1

//mesangem só em caso de erro
if (p == (void *) -1) {
        perror("Erro ao acoplar memória (shmat)");
        exit(1);
    }

printf("(Cliente // pid %d): %d\n",getpid(),*p);

// não sabia se precisa fazer ou não mas fiz
shmdt(p);

return 0;
}




