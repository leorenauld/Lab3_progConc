#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"

float *A, *B, *C;
int N, M, P, nthreads;

typedef struct {
    int id;
    int start;
    int end;
} tArgs;

void* tarefa(void* arg) {
    tArgs *args = (tArgs*) arg;
    for (int i = args->start; i < args->end; i++) {
        for (int j = 0; j < P; j++) {
            C[i * P + j] = 0;
            for (int k = 0; k < M; k++) {
                C[i * P + j] += A[i * M + k] * B[k * P + j];
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        printf("Usage: %s <matrizA> <matrizB> <matrizC> <nthreads>\n", argv[0]);
        return 1;
    }

    FILE *fileA = fopen(argv[1], "rb");
    FILE *fileB = fopen(argv[2], "rb");
    FILE *fileC = fopen(argv[3], "wb");
    nthreads = atoi(argv[4]);

    if (!fileA || !fileB || !fileC) {
        printf("Erro na abertura dos arquivos.\n");
        return 1;
    }

    int temp;
    fread(&N, sizeof(int), 1, fileA);
    fread(&M, sizeof(int), 1, fileA);
    fread(&temp, sizeof(int), 1, fileB);
    fread(&P, sizeof(int), 1, fileB);

    if (M != temp) {
        printf("Dimensões incompatíveis para multiplicação.\n");
        return 1;
    }

    A = (float*) malloc(N * M * sizeof(float));
    B = (float*) malloc(M * P * sizeof(float));
    C = (float*) malloc(N * P * sizeof(float));

    fread(A, sizeof(float), N * M, fileA);
    fread(B, sizeof(float), M * P, fileB);

    fclose(fileA);
    fclose(fileB);

    pthread_t *tid = (pthread_t*) malloc(nthreads * sizeof(pthread_t));
    tArgs *args = (tArgs*) malloc(nthreads * sizeof(tArgs));

    double inicio, fim;
    GET_TIME(inicio);

    int blockSize = N / nthreads;
    for (int i = 0; i < nthreads; i++) {
        args[i].id = i;
        args[i].start = i * blockSize;
        args[i].end = (i == nthreads - 1) ? N : (i + 1) * blockSize;
        if (pthread_create(&tid[i], NULL, tarefa, (void*) &args[i])) {
            printf("Erro na criação da thread %d\n", i);
            return 3;
        }
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
    }

    GET_TIME(fim);
    printf("Tempo de execução concorrente: %lf\n", fim - inicio);

    fwrite(&N, sizeof(int), 1, fileC);
    fwrite(&P, sizeof(int), 1, fileC);
    fwrite(C, sizeof(float), N * P, fileC);

    fclose(fileC);

    free(A);
    free(B);
    free(C);
    free(tid);
    free(args);

    return 0;
}
