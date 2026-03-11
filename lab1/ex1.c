#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int main(void){
int mypid, pid, status;
pid = fork();

int meupid= getpid(); 
printf("Pid: %d\n", meupid);

if (pid!=0) { //Pai
waitpid(-1, &status, 0);
}
else { //Filho
exit(3);
}
return 0;
}