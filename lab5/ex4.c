#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXFILA 8
#define TOTAL_ELEMENTOS 64
#define NUM_PRODUTORES 2
#define NUM_CONSUMIDORES 2

int buffer[MAXFILA];
int count = 0, in = 0, out = 0;
int produzidos = 0, consumidos = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;

void* produtor(void* arg) {
    int id = (int)(long)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        if (produzidos >= TOTAL_ELEMENTOS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        while (count == MAXFILA) {
            pthread_cond_wait(&can_produce, &mutex);
        }

        int item = (rand() % 100) + 1;
        buffer[in] = item;
        produzidos++;
        printf("Produtor %d: Inseriu %d na pos %d (%d/%d)\n", id, item, in, produzidos, TOTAL_ELEMENTOS);
        
        in = (in + 1) % MAXFILA;
        count++;

        pthread_cond_broadcast(&can_consume);
        pthread_mutex_unlock(&mutex);
        sleep(1);
    }
    return NULL;
}

void* consumidor(void* arg) {
    int id = (int)(long)arg;
    while (1) {
        pthread_mutex_lock(&mutex);
        if (consumidos >= TOTAL_ELEMENTOS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        while (count == 0) {
            pthread_cond_wait(&can_consume, &mutex);
        }

        int item = buffer[out];
        consumidos++;
        printf("Consumidor %d: Retirou %d da pos %d (%d/%d)\n", id, item, out, consumidos, TOTAL_ELEMENTOS);
        
        out = (out + 1) % MAXFILA;
        count--;

        pthread_cond_broadcast(&can_produce);
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    return NULL;
}

int main() {
    pthread_t prods[NUM_PRODUTORES], cons[NUM_CONSUMIDORES];
    for (long i = 0; i < NUM_PRODUTORES; i++) pthread_create(&prods[i], NULL, produtor, (void*)i);
    for (long i = 0; i < NUM_CONSUMIDORES; i++) pthread_create(&cons[i], NULL, consumidor, (void*)i);
    for (int i = 0; i < NUM_PRODUTORES; i++) pthread_join(prods[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) pthread_join(cons[i], NULL);
    return 0;
}