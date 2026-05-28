#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define DB_SIZE 1000
#define CACHE_SIZE 100

#define DB_SUCCESS 1
#define DB_ERROR 0

// --- Estruturas e Variáveis Globais ---

typedef struct {
    int key;
    int value;
    time_t timestamp;
} CacheEntry;

CacheEntry cache[CACHE_SIZE];
int is_valid[CACHE_SIZE] = {0};
int banco_de_dados[DB_SIZE];

// --- Tratamento de Sinal (Ctrl-Z) ---

void handle_sigtstp(int sig) {
    printf("\n\n>>> [SINAL] Ctrl-Z capturado! Limpando todo o cache... <<<\n\n");
    for (int i = 0; i < CACHE_SIZE; i++) {
        is_valid[i] = 0;
    }
}

// --- Funções Simuladas do Banco de Dados ---

int db_read(int key, int* out_value) {
    if (key < 0 || key >= DB_SIZE) return DB_ERROR;
    *out_value = banco_de_dados[key];
    return DB_SUCCESS;
}

int db_write(int key, int new_value) {
    if (key < 0 || key >= DB_SIZE) return DB_ERROR;
    banco_de_dados[key] = new_value;
    return DB_SUCCESS;
}



int main() {
    for (int i = 0; i < DB_SIZE; i++) {
        banco_de_dados[i] = i + 1;
    }


    signal(SIGTSTP, handle_sigtstp);


    FILE *fp = fopen("log.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro ao abrir log.txt. Execute log_gen primeiro.\n");
        return 1;
    }

    printf("Iniciando processamento (Cache-Aside). Pressione Ctrl-Z para limpar o cache.\n\n");

    char line[128];
    int key, value, val_out;


    while (1) {
        
        if (fgets(line, sizeof(line), fp) == NULL) {
            rewind(fp);
            continue;
        }

       

        if (line[0] == 'R') {
            sscanf(line, "R, %d", &key);
            int idx = key % CACHE_SIZE; 

           
            if (is_valid[idx] && cache[idx].key == key) {
                printf("[READ] Key %3d -> CACHE HIT!  (Valor: %d)\n", key, cache[idx].value);
                cache[idx].timestamp = time(NULL); 
            } else {
                printf("[READ] Key %3d -> CACHE MISS! Buscando no DB...\n", key);
                db_read(key, &val_out);
                
                
                cache[idx].key = key;
                cache[idx].value = val_out;
                cache[idx].timestamp = time(NULL);
                is_valid[idx] = 1;
            }

        } else if (line[0] == 'W') {
            sscanf(line, "W, %d, %d", &value, &key);
            int idx = key % CACHE_SIZE;

            printf("[WRITE] Key %3d -> Gravando direto no DB (Novo valor: %d)\n", key, value);
            db_write(key, value);

           
            if (is_valid[idx] && cache[idx].key == key) {
                is_valid[idx] = 0;
                printf("        -> Entrada invalidada no cache para manter consistência.\n");
            }
        }

       
        usleep(300000); 
    }

    fclose(fp);
    return 0;
}