#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXFILA 8
#define TOTAL_ELEMENTOS 64


int buffer[MAXFILA];
int count = 0; 
int in = 0;    
int out = 0;   


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;

void* produtor(void* arg) {
    for (int i = 1; i <= TOTAL_ELEMENTOS; i++) {
        pthread_mutex_lock(&mutex); 

        
        while (count == MAXFILA) {
            printf("Produtor: Fila cheia. Aguardando...\n");
            pthread_cond_wait(&can_produce, &mutex);
        }

        
        int item = (rand() % 100) + 1;
        buffer[in] = item;
        printf("Produtor: Inseriu %d na posicao %d (Total na fila: %d/64)\n", item, in, i);
        
        in = (in + 1) % MAXFILA; 
        count++;

        
        pthread_cond_signal(&can_consume);
        pthread_mutex_unlock(&mutex); 

        sleep(1); 
    }
    pthread_exit(NULL);
}

void* consumidor(void* arg) {
    for (int i = 1; i <= TOTAL_ELEMENTOS; i++) {
        pthread_mutex_lock(&mutex);

        
        while (count == 0) {
            printf("Consumidor: Fila vazia. Aguardando...\n");
            pthread_cond_wait(&can_consume, &mutex);
        }

        
        int item = buffer[out];
        printf("Consumidor: Retirou %d da posicao %d (Total consumido: %d/64)\n", item, out, i);
        
        out = (out + 1) % MAXFILA; 
        count--;

        
        pthread_cond_signal(&can_produce);
        pthread_mutex_unlock(&mutex);

        sleep(2); 
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread_prod, thread_cons;

    printf("Iniciando problema do Produtor/Consumidor...\n");

    
    pthread_create(&thread_prod, NULL, produtor, NULL);
    pthread_create(&thread_cons, NULL, consumidor, NULL);

    
    pthread_join(thread_prod, NULL);
    pthread_join(thread_cons, NULL);

    printf("Simulacao concluida com sucesso.\n");

    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_produce);
    pthread_cond_destroy(&can_consume);

    return 0;
}