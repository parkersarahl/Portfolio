#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define DEFAULT_MAX_THREADS 10
#define MIN_THREADS 1
#define MAXVALUE  99999999999999999 // Maximum value for random long int generation
#define MAX_TRIES 10 // Maximum number of attempts for each thread

/* Structure to pass arguments to threads */
typedef struct {
    int thread_id;
} ThreadArgs;

/* Structure to store results */
typedef struct {
    int thread_id;
    long int number;
    bool is_prime;
} ThreadResult;

/* Array to store results */
ThreadResult results[DEFAULT_MAX_THREADS];

/* Mutex for thread safety */
pthread_mutex_t mutex;

/* Function to check if a number is prime */
bool is_prime(long int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

/* Pthreads entry point */
void* threadfunc(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int thread_id = args->thread_id;
    long int number;
    bool found_prime = false;

    for (int attempt = 0; attempt < MAX_TRIES; attempt++) {
        number = rand() % MAXVALUE + 1; // Generate a new random number for each attempt

        /* Check if the number is prime */
        if (is_prime(number)) {
            found_prime = true;

            /* Lock the mutex before writing to results */
            pthread_mutex_lock(&mutex);
            results[thread_id - 1].thread_id = thread_id;
            results[thread_id - 1].number = number;
            results[thread_id - 1].is_prime = true;
            pthread_mutex_unlock(&mutex);
            break; // Exit the loop if a prime is found
        }
    }

    // If no prime found after max attempts
    if (!found_prime) {
        pthread_mutex_lock(&mutex);
        results[thread_id - 1].thread_id = thread_id;
        results[thread_id - 1].number = number; // Last checked number
        results[thread_id - 1].is_prime = false;
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int num_threads = DEFAULT_MAX_THREADS;

    /* Parse command-line arguments */
    if (argc > 1) {
        num_threads = atoi(argv[1]);
        if (num_threads <= 0) {
            fprintf(stderr, "Invalid number of threads. Must be a positive integer.\n");
            return 1;
        }
    }
    printf("At start\n");

    /* Initialize random seed */
    srand(time(NULL));

    /* Print number of threads from argument on command line */
    printf("Number of threads to use: %d\n", num_threads);

    /* Initialize the mutex */
    pthread_mutex_init(&mutex, NULL);

    /* Stack-allocated thread ids and individual thread arguments */
    pthread_t tid[num_threads];
    ThreadArgs thread_args[num_threads];

    /* Create and assign a unique thread ID for each thread */
    for (int i = 0; i < num_threads; i++) {
        thread_args[i].thread_id = i + 1;  // Sequential thread ID starting from 1
    }

    /* Initialize results array */
    for (int i = 0; i < num_threads; i++) {
        results[i].thread_id = 0;  // Initialize with default values
    }

    /* Create threads */
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&tid[i], NULL, threadfunc, &thread_args[i]);
    }

    printf("Waiting for threads to finish\n");

    /* Join threads */
    for (int i = 0; i < num_threads; i++) {
        pthread_join(tid[i], NULL);
    }

    /* Print results in sequential order of thread number */
    printf("Results:\n");
    for (int i = 0; i < num_threads; i++) {
        printf("Thread %d reports ", results[i].thread_id); 
        printf(results[i].is_prime ? "%ld is a prime\n" : "no prime\n", results[i].number);
    }

    /* Destroy the mutex */
    pthread_mutex_destroy(&mutex);

    return 0;
}
