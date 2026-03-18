#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>


int main ()
{
// comen tarios importantes: Vou usar matrizes 2x2 e vou seguir com a segunte logica os dois primeiros calores forma a primeira linha e os dois outros a segunda linha 
// ou seja v[4]= {1,2,3,4} forma a seguinte matriz:
//|1,2|
//|3,4|


int matriz1,matriz2,matriz3, *p1,*p2,*p3, id, pid, status, pid2;
int linhas, colunas;

linhas =2;
colunas=2;


//MATRIZ 1 (PREENCHIDA) --------------------------------------------------------------------

matriz1 = shmget (IPC_PRIVATE, sizeof (int)*4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p1 = (int *) shmat (matriz1, 0, 0); // comparar o retorno com -1
p1[0] =1;
p1[1] =1;
p1[2] =1;
p1[3] =1;


//MATRIZ 2 (PREENCHIDA)-------------------------------------------------------------------------
matriz2 = shmget (IPC_PRIVATE, sizeof (int)*4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p2 = (int *) shmat (matriz2, 0, 0); // comparar o retorno com -1
p2[0] =0;
p2[1] =0;
p2[2] =0;
p2[3] =0;


// MATRIZ 3 (VAZIA)-------------------------------------------------------------------
matriz3 = shmget (IPC_PRIVATE, sizeof (int)*4, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);


p3 = (int *) shmat (matriz3, 0, 0); // comparar o retorno com -1



if ((id = fork()) < 0)
{
puts ("Erro na criação do novo processo");
exit (3);
}

//Filho

for (size_t i = 0; i < linhas; i++)
{
    pid=fork();
    if (id == 0){
        for (size_t i = 0; i < colunas; i++)
        {
        p3[i]= p1[i]+p2[i];
        printf("%d ", p3[i]);
        }
        printf("\n");
        exit(3);
    }  
    
}
waitpid(-1, NULL, 0);


// libera a memória compartilhada do processo
shmdt (p1);
shmdt (p2);
shmdt (p3);
// libera a memória compartilhada
shmctl (matriz1, IPC_RMID, 0);
shmctl (matriz2, IPC_RMID, 0);
shmctl (matriz3, IPC_RMID, 0);
return 0;

}

//Dicas do Monitor
//for l in linhas:
//    f = fork()
//    if filho:
//        trabalha na linha l
//        exit



