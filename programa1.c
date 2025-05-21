#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

// la guia del lab mencionaba que con "ajustable" era posible cambiar los valores
// de un define y que no era necesario hacer que el usuario elija :)
#define NUM_THREADS 5
#define NUM_ITERATIONS 3
#define INITIAL_RESOURCES 10

// globales
int available_resources = INITIAL_RESOURCES;
FILE *log_file;
sem_t resource_sem;

// par los los logs
void log_message(const char *message) {
    fprintf(log_file, "%s\n", message);
    fflush(log_file);
}

void* thread_function(void* arg) {
    long thread_id = (long)arg;
    char log_msg[100];
    
    sprintf(log_msg, "Thread %ld iniciado", thread_id);
    log_message(log_msg);
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sprintf(log_msg, "Thread %ld - Iteración %d", thread_id, i+1);
        log_message(log_msg);
        
        // consumir
        sem_wait(&resource_sem);
        available_resources--;
        
        sprintf(log_msg, "Thread %ld - Recurso tomado. Recursos disponibles: %d", 
                thread_id, available_resources);
        log_message(log_msg);
        
        // tiemp trabajo medio random 
        int sleep_time = rand() % 3 + 1;
        sleep(sleep_time);
        
        // devolver
        available_resources++;
        sem_post(&resource_sem);
        
        sprintf(log_msg, "Thread %ld - Recurso devuelto. Recursos disponibles: %d", 
                thread_id, available_resources);
        log_message(log_msg);
    }
    
    return NULL;
}

int main() {
    srand(time(NULL));
    log_file = fopen("programa1.txt", "w");
    if (!log_file) {
        perror("Error al abrir archivo de log");
        return 1;
    }
    
    log_message("Iniciando programa");


    sem_init(&resource_sem, 0, INITIAL_RESOURCES);
    pthread_t threads[NUM_THREADS];

    //creacion de los threads
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void*)i);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    //limpiar
    sem_destroy(&resource_sem);
    fclose(log_file);
    
    log_message("Programa terminado");
    return 0;
}