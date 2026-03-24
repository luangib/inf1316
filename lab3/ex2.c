#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define EVER ;;
void killHandler(int sinal);
int main (void)
{
void (*p)(int); // ponteiro para função que recebe int como
// parâmetro
printf("Pid do processo: %d\n", getpid());
p = signal(SIGKILL, SIG_IGN);
printf("Endereco do manipulador anterior %p\n", p);
puts ("Tentando Desabilitar o kill");
for(EVER);
}
// vou passar kill –s SIGKILL <PID>, com o pid que aparecer no print