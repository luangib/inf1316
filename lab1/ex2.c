#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
int mypid, pid, status;
pid = fork();
int n=1;

int meupid= getpid(); 
printf("Pid: %d\n", meupid);

if (pid!=0) { //Pai
    printf("Pai: %d\n", n);
    waitpid(-1, &status, 0);
    printf("Pai: %d\n", n);
}
else { //Filho
    n=2;
    printf("Filho: %d\n",n);
    exit(3);
}
return 0;
}