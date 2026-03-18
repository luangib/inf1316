//ex2.c

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
int segmento, *p, id, pid, status;
int n;

//processo da memoria compartilhada
segmento = shmget (8752, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

p = (int *) shmat (segmento, 0, 0); // comparar o retorno com -1


//pegando algo pelo stin
printf("(Pai // Pid: %d) Digite um numero inteiro:",getpid());
fscanf(stdin,"%d", p);

pid = fork();

if(pid==0){
//Prefiri tranformar um filho no cliente. Para só chamar um programa.
execl("./cliente", "cliente",NULL);
exit(0);

}
else{
waitpid(-1, NULL, 0);

//Tirando a memoria alocada
shmdt(p);
shmctl(segmento,IPC_RMID,NULL);
}

return 0;
}




