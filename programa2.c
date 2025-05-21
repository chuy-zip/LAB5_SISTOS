#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// igual que en el programa 1,
// la guia del lab mencionaba que con "ajustable" era posible cambiar los valores
// de un define y que no era necesario hacer que el usuario elija :)
#define NUM_THREADS 5
#define NUM_ITERATIONS 3

// Nota importante: si se ponen menos de 3 recursos el programa se puede quedar loopeado
// en las innstrucciones no especifica la cantidad que consumen los threads
// entonces puse que consuman de 1-3 de forma aleatoria, cual implica que si se pone
// menos de 3 y algun thread solicita 3, se quedará esperando indefinidamente
// osea, dejar 3 o más sino... pum
#define INITIAL_RESOURCES 10

// Variables globales
int available_resources = INITIAL_RESOURCES;
FILE *log_file;

// Este es el workaround para hacer un monitor, usar condiciones mutex y boradcast
// para simular la espera en el monitor. el boradcast es la senial que mandan los threads para notificar 
// sobre la disponibilidad del recurso
pthread_mutex_t monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t resources_available = PTHREAD_COND_INITIALIZER;

// Función para el log
void log_message(const char *message) {
    fprintf(log_file, "%s\n", message);
    fflush(log_file);
}

int decrease_count(int count) {
    pthread_mutex_lock(&monitor_mutex);
    char log_msg[100];
    
    //Cuando una pthread se encuentra dentro de la funcion, quiere decir que intenta accedar
    //al recurso
    sprintf(log_msg, "Iniciando decrease_count");
    log_message(log_msg);
    sprintf(log_msg, "Thread %ld ha adquirido el Mutex, entrando al monitor en decrease", (long)pthread_self());
    log_message(log_msg);

    //pero puede que seda temporalmente si no hay recursos suficientes para lo que quiere consumir y espera
    // De no hacer esto pasaba que un thread se quedaba esperando con el mutex de los recursos
    // y los demas, aunque consumieran menos y si puedieran consumir, tenian que esperar también
    while (available_resources < count) {
        sprintf(log_msg, "Thread %ld Recursos insuficientes. Esperando. Disponibles: %d, Requeridos: %d", 
                (long)pthread_self(), available_resources, count);
        log_message(log_msg);
        
        pthread_cond_wait(&resources_available, &monitor_mutex);
    }
    
    //habiendo salido del while quiere decir que los recursos ya estan disponibles
    //y sale del wait. Etonces se reducen los recursos de forma segura
    available_resources -= count;
    sprintf(log_msg, "Thread %ld ha consumido %d recursos. Recursos disponibles ahora: %d", (long)pthread_self(), count, available_resources);
    log_message(log_msg);
    sprintf(log_msg, "Terminando decrease_count");
    log_message(log_msg);
    
    pthread_mutex_unlock(&monitor_mutex);
    return 0;
}

int increase_count(int count) {
    pthread_mutex_lock(&monitor_mutex);
    char log_msg[100];
    

    sprintf(log_msg, "Thread %ld - Entrando a increase_count(%d)", (long)pthread_self(), count);
    log_message(log_msg);
    
    available_resources += count;
    
    sprintf(log_msg, "Thread %ld ha devuelto %d recursos. Recursos disponibles ahora: %d", (long)pthread_self(), count, available_resources);
    log_message(log_msg);
    
    // Notificar a todos los threads que podrían estar esperando recursos, aqui es
    // cuando los pthreads avisan a los demas pthreads en espera
    pthread_cond_broadcast(&resources_available);
    pthread_mutex_unlock(&monitor_mutex);
    return 0;
}

void* thread_function(void* arg) {
    long thread_id = (long)arg;
    char log_msg[100];
    
    sprintf(log_msg, "Thread %ld iniciado", (long)pthread_self());
    log_message(log_msg);
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        sprintf(log_msg, "Thread %ld - Iteración %d", (long)pthread_self(), i+1);
        log_message(log_msg);
        
        // Consumir entre 1-3 recursos aleatoriamente, no decía cantidad entonces asumi que era aleatorio
        // para no dejar valores ahrdcoded
        int resources_to_take = rand() % 3 + 1;
        sprintf(log_msg, "Thread %ld - Intentando consumir %d recursos", (long)pthread_self(), resources_to_take);
        log_message(log_msg);
        
        decrease_count(resources_to_take);
        
        // Simular trabajo
        int sleep_time = rand() % 3 + 1;
        sleep(sleep_time);
        
        // Devolver recursos luego de cumplido el tiempo, 
        // pero los demas threads seguiran intentnado acceder mientras
        increase_count(resources_to_take);
    }
    
    return NULL;
}

int main() {
    srand(time(NULL));
    log_file = fopen("programa2.txt", "w");
    if (!log_file) {
        perror("Error al abrir archivo de bitácora");
        return 1;
    }
    
    log_message("Iniciando programa con monitor");
    
    pthread_t threads[NUM_THREADS];
    
    // Crear threads
    for (long i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_function, (void*)i);
    }
    
    // Esperar a que todos los threads terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Limpieza
    pthread_mutex_destroy(&monitor_mutex);
    pthread_cond_destroy(&resources_available);
    
    log_message("Programa terminado");

    fclose(log_file);
    
    return 0;
}