#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 5 
#define PULO_MAXIMO 100 
#define DESCANSO_MAXIMO 2
#define DISTANCIA_PARA_CORRER 500 


static int classificacao = 1;
static pthread_mutex_t lock;

void *Correr(void *sapo) {
    int pulos = 0;
    int distanciaJaCorrida = 0; 
    int id_sapo = (int)(long)sapo;

    while (distanciaJaCorrida <= DISTANCIA_PARA_CORRER) { 
        int pulo = rand() % PULO_MAXIMO; 
        distanciaJaCorrida += pulo; 
        pulos++;

        printf("Sapo %d pulou %d cm (Total: %d cm)\n", id_sapo, pulo, distanciaJaCorrida);
        
        int descanso = rand() % DESCANSO_MAXIMO; 
        sleep(descanso); 
    }

    pthread_mutex_lock(&lock); 
    
    printf("\n>>> Sapo %d CHEGOU! Posicao: %d (Total de pulos: %d)\n\n", 
            id_sapo, classificacao, pulos); 
    
    classificacao++; 
    
    pthread_mutex_unlock(&lock); 
    

    pthread_exit(NULL); 
}

int main() {
    pthread_t threads[NUM_THREADS]; 
    int t;

    
    pthread_mutex_init(&lock, NULL);

    printf("Corrida iniciada ... \n"); 

    for(t = 0; t < NUM_THREADS; t++) { 
        
        pthread_create(&threads[t], NULL, Correr, (void *)(long)t); 
    }

    for(t = 0; t < NUM_THREADS; t++) { 
        
        pthread_join(threads[t], NULL); 
    }

    printf("\nAcabou!!\n"); 
    
    
    pthread_mutex_destroy(&lock);

    return 0;
}