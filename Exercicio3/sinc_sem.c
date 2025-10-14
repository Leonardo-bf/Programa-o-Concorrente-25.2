#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <semaphore.h>

long long int *numeros;// Números gerados pela produtora
long long int *buffer;// Canal de Inteiros
int *primosPorThread;// Contador de primos por thread consumidora

long long int N; //Sequência de números inteiros
int M, C;
int in = 0, out = 0;
int numerosConsumidos = 0;

pthread_mutex_t mutex;
sem_t cheio, vazio;

int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (long long int i = 3; i < sqrt(n) + 1; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void *ExecutaProdutora(void *arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&vazio);//decrementa o semáforo
        pthread_mutex_lock(&mutex);//inicio da seção crítica
        
        buffer[in] = numeros[i]; 
        in = (in + 1) % M;
        
        pthread_mutex_unlock(&mutex);//fim da seção crítica
        sem_post(&cheio);//incrementa o semáforo
    }
    pthread_exit(NULL);
}


void *ExecutaConsumidora(void *arg) {
    long int id = (long int)arg;
    long long int valor; 

    while (1) {
        sem_wait(&cheio);//decrementa o semáforo

        pthread_mutex_lock(&mutex);//inicio da seção crítica


        if (numerosConsumidos >= N) {
            pthread_mutex_unlock(&mutex);//fim da seção crítica
            break; 
        }

        valor = buffer[out];
        out = (out + 1) % M;
        numerosConsumidos++;

        pthread_mutex_unlock(&mutex);//fim da seção crítica
        sem_post(&vazio);//incrementa o semáforo

        if (ehPrimo(valor)) {
            pthread_mutex_lock(&mutex);//inicio seção crítica
            primosPorThread[id]++;
            pthread_mutex_unlock(&mutex);//fim da seção crítica
        }
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <N> <M> <C>\n", argv[0]);
        return 1;
    }

    N = atoll(argv[1]);
    M = atoi(argv[2]);
    C = atoi(argv[3]);


    numeros = (long long int *)malloc(sizeof(long long int) * N);
    buffer = (long long int *)malloc(sizeof(long long int) * M);
    primosPorThread = (int *)calloc(C, sizeof(int));
    pthread_t *tid_consumidores = (pthread_t *)malloc(sizeof(pthread_t) * C);
    pthread_t tid_produtora;

    if (!numeros || !buffer || !primosPorThread || !tid_consumidores) {
        printf("Erro de alocação de memória\n");
        return 2;
    }


    srand(time(NULL));
    int contador = rand() % 100;
    for (int i = 0; i < N; i++) {
        numeros[i] = contador++;
    }

    // Inicializa mutex e semáforos
    pthread_mutex_init(&mutex, NULL);
    sem_init(&cheio, 0, 0);
    sem_init(&vazio, 0, M);

    if (pthread_create(&tid_produtora, NULL, ExecutaProdutora, NULL)) {
        printf("--ERRO: pthread_create() produtora\n");
        exit(-1);
    }

    for (long int t = 0; t < C; t++) {
        if (pthread_create(&tid_consumidores[t], NULL, ExecutaConsumidora, (void *)t)) {
            printf("--ERRO: pthread_create() consumidora %ld\n", t);
            exit(-1);
        }
    }

    if (pthread_join(tid_produtora, NULL)) {
        printf("--ERRO: pthread_join() produtora\n");
        exit(-1);
    }

    for (int i = 0; i < C; i++) {
        sem_post(&cheio);
    }

    for (int t = 0; t < C; t++) {
        if (pthread_join(tid_consumidores[t], NULL)) {
            printf("--ERRO: pthread_join() consumidora %d\n", t);
            exit(-1);
        }
    }

    // Destrói mutex e semáforos
    pthread_mutex_destroy(&mutex);
    sem_destroy(&cheio);
    sem_destroy(&vazio);

    int totalPrimos = 0, vencedora = 0;
    for (int i = 0; i < C; i++) {
        totalPrimos += primosPorThread[i];
        if (primosPorThread[i] > primosPorThread[vencedora]) {
            vencedora = i;
        }
    }

    printf("\nTotal de primos encontrados: %d\n", totalPrimos);
    printf("Thread consumidora vencedora: %d (com %d primos)\n", vencedora, primosPorThread[vencedora]);
    for(int i=0; i<N; i++){
        printf("%lld ", numeros[i]);
    }
    printf("\n");

    free(numeros);
    free(buffer);
    free(primosPorThread);
    free(tid_consumidores);

    return 0;
}
