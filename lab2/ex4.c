//ex4.c

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
//Prefiri puxar um numero em vez de uma mesagem do dia para deixar mais simples, não acho que vá alterar muita coisa(além do espaço alocado na memória compartilhada)

//minhas variaveis 
int segmento1, segmento2, *p1, *p2, id, pid, status;
int n;

//processo da criação m1 
segmento1 = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p1 = (int *) shmat (segmento1, 0, 0); // comparar o retorno com -1

//processo de criação m2
segmento2 = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p2 = (int *) shmat (segmento2, 0, 0); // comparar o retorno com -1


//pegando algo pelo stin
printf("(Pai // Pid: %d) Inicio",getpid());


pid = fork();

if(pid==0){
execl("./p1", "p1",NULL);
exit(0);

}
else{
waitpid(-1, NULL, 0);

//Tirando a memoria alocada
shmdt(p1);
shmctl(segmento1,IPC_RMID,NULL);

shmdt(p2);
shmctl(segmento2,IPC_RMID,NULL);
}

return 0;
}




