#include <stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include"timer.h"
#include<time.h>

long int n; //dimensao do vetor
float *vet1, *vet2; //vetores de elemntos
int nthreads;//numero de threads


//função executada pelas threads
void *Produto_Interno (void *tid){
    long int id = (long int) tid;
    int ini, bloco, fim;
    float prod_int=0, *ret;

    //realiza a separação dos blocos das threads
    bloco = n / nthreads;
    ini = id * bloco;
    fim = ini + bloco;

    if(id == (nthreads-1)) fim = n;

    //Realiza o calculo do produto interno do bloco da thread
    for(int i = ini; i<fim; i++){
        prod_int = prod_int + (vet1[i] * vet2[i]);
    }

    ret = (float*)malloc(sizeof(float));
    if(ret!=NULL) *ret = prod_int;
    else printf("ERRO: não foi possível alocar ret usando malloc\n");
    pthread_exit((void*) ret);

}

//função main
int main(int argc, char *argv[]){
    FILE * arq;
    size_t ret;
    double prod_int_seq;
    double variancia;
    float prod_int_global;
    float *prod_int_funcao;
    double start, end, tempo_de_execucao;

    pthread_t *tid;


    //abre o arquivo de entrada com os vetores para o calculo do produto interno
    arq = fopen(argv[1], "rb");
    if(arq==NULL) { 
        printf("--ERRO: fopen()\n"); 
        exit(-1); 
    }

    //verifica se o programa foi iniciado corretamente
    if(argc < 3){
        printf("Use: %s <arquivo de entrada> <numero de threads> \n", argv[0]);
        exit(-1);
    }

    ret = fread(&n, sizeof(long int), 1, arq);
    //verifica a leitura do arquivo
    if(!ret) {
       fprintf(stderr, "Erro de leitura das dimensoes da matriz arquivo \n");
       return 3;
    }
  
    //aloca espaco de memoria e carrega os vetores de entrada
    vet1 = (float*) malloc (sizeof(float) * n);
    vet2 = (float*) malloc (sizeof(float) * n);
    if(vet1 == NULL || vet2 == NULL) { 
        printf("--ERRO: malloc()\n"); 
        exit(-1); 
    }
    ret = fread(vet1, sizeof(float), n, arq);
    ret = fread(vet2, sizeof(float), n, arq);

    if(ret < n) {
       fprintf(stderr, "Erro de leitura dos elementos do vetor\n");
       return 4;
    }
  
    //le o numero de threads da entrada do usuario 
    nthreads = atoi(argv[2]);
    //limita o numero de threads ao tamanho do vetor
    if(nthreads>n) nthreads = n;
  
    //aloca espaco para o vetor de identificadores das threads no sistema
    tid = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    if(tid==NULL) { 
        printf("--ERRO: malloc()\n"); 
        exit(-1); 
    }
    GET_TIME(start);
    //cria as threads
    for(long int i=0; i<nthreads; i++) {
      if (pthread_create(&tid[i], NULL, Produto_Interno, (void*) i)) {
         printf("--ERRO: pthread_create()\n"); exit(-1);
      }
    }
    //espera todas as threads terminarem e calcula a soma total das threads
    prod_int_global=0;
    for(int i=0; i<nthreads; i++) {
       if (pthread_join(tid[i], (void *) &prod_int_funcao)) {
          printf("--ERRO: pthread_join()\n"); exit(-1);
       }
       prod_int_global += *prod_int_funcao;
       free(prod_int_funcao);
    }
    GET_TIME(end);
    //imprime os resultados
    printf("\n");
    printf("Produto Interno Concorrente = %.26f\n", prod_int_global);
    //le o produto interno registrado no arquivo
    ret = fread(&prod_int_seq, sizeof(double), 1, arq); 
    printf("\nProduto Interno Sequencial = %.26lf\n", prod_int_seq);

    variancia = (prod_int_seq - prod_int_global) / prod_int_seq;
    printf("\nVariância Relativa = %.10lf\n", variancia);
  
    tempo_de_execucao = end - start;
    printf("Tempo de execução para %d threads foi igual a %lf\n", nthreads, tempo_de_execucao);
    //desaloca os espacos de memoria
    free(vet1);
    free(vet2);
    free(tid);
    //fecha o arquivo
    fclose(arq);
    return 0;
}
