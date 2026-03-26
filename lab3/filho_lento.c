//filho_lento.c

#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Filho: Começando execução longa (20s)...\n");
    sleep(20);
    printf("Filho: Isso não deve aparecer se o pai me matar!\n");
    return 0;
}