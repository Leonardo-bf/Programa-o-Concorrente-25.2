/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


long int soma = 0;             // variável compartilhada entre as threads
pthread_mutex_t mutex;         // //variavel de lock para exclusao mutua
pthread_cond_t cond;           // variável de condição
int finalizar = 0;             // flag para indicar fim
long int ultimo_impresso = 0;  // último múltiplo de 1000 impresso

// função executada pelas threads de trabalho
void *ExecutaTarefa(void *arg) {
    long int id = (long int) arg;
    printf("Thread %ld: executando...\n", id);

    for (int i = 0; i < 100000; i++) {
        pthread_mutex_lock(&mutex);

        soma++;

        // quando atingir novo múltiplo de 1000, acorda thread extra
        if (soma % 1000 == 0) {
            pthread_cond_signal(&cond);
        }

        pthread_mutex_unlock(&mutex);
    }

    printf("Thread %ld: terminou!\n", id);
    pthread_exit(NULL);
}

// função executada pela thread extra
void *extra(void *args) {
    printf("Extra: executando...\n");

    pthread_mutex_lock(&mutex);

    while (!finalizar) {
        // espera até ter pelo menos um novo múltiplo de 1000 para imprimir
        while ((soma < ultimo_impresso + 1000) && !finalizar) {
            pthread_cond_wait(&cond, &mutex);
        }

        if (finalizar) break;

        // imprime todos os múltiplos de 1000 que ainda não foram impressos
        while (soma >= ultimo_impresso + 1000) {
            ultimo_impresso += 1000;
            printf("soma = %ld\n", ultimo_impresso);
        }
    }

    pthread_mutex_unlock(&mutex);

    printf("Extra: terminou!\n");
    pthread_exit(NULL);
}

// fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; // identificadores das threads no sistema
    int nthreads;   // qtde de threads (passada na linha de comando)

    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    tid = (pthread_t *) malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) {
        puts("ERRO--malloc");
        return 2;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // cria as threads de trabalho
    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    // cria a thread extra
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    // espera todas as threads de trabalho terminarem
    for (int t = 0; t < nthreads; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join()\n");
            exit(-1);
        }
    }

    // sinaliza para a thread extra que pode finalizar
    pthread_mutex_lock(&mutex);
    finalizar = 1;
    pthread_cond_signal(&cond); // acorda a thread extra para sair
    pthread_mutex_unlock(&mutex);

    pthread_join(tid[nthreads], NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Valor final de 'soma' = %ld\n", soma);

    free(tid);
    return 0;
}
