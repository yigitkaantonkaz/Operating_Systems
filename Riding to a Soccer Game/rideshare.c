#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>

pthread_mutex_t lock_t = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier_t;

struct thread_{
    char team;
    sem_t * sem_a;
    sem_t * sem_b;
    int * num_a;
    int * num_b;
};

void * pthread_(void * args){
    struct thread_ * arg = (struct thread_*) args;
    bool car = 0;
   
    pthread_mutex_lock(&lock_t);
    printf("Thread ID: %ld, Team: %c, I am looking for a car\n", pthread_self(), arg->team);
    
    if(arg->team == 'A'){
        *arg->num_a = *arg->num_a + 1;
    }
    else{
        *arg->num_b = *arg->num_b + 1;
    }

    
    if(arg->team == 'A'){
        if(*arg->num_a == 4){
            for(int i = 0; i < 3; ++i){
                sem_post(arg->sem_a);
            }
            car = 1;
            *arg->num_a = *arg->num_a - 4;
        }
        else if(*arg->num_b > 1 && *arg->num_a == 2){
            sem_post(arg->sem_a);
            for (int i = 0; i < 2; ++i){
                sem_post(arg->sem_b);
            }
            car = 1;
            *arg->num_a = *arg->num_a - 2;
            *arg->num_b = *arg->num_b - 2;
        }
        else{
            pthread_mutex_unlock(&lock_t);
            sem_wait(arg->sem_a);
        }
    }else if (arg->team == 'B'){
        if(*arg->num_b == 4){
            for(int i = 0; i < 3; ++i){
                sem_post(arg->sem_b);
            }
            car = 1;
            *arg->num_b = *arg->num_b - 4;
        }
        else if(*arg->num_a > 1 && *arg->num_b == 2){
            sem_post(arg->sem_b);
            for (int i = 0; i < 2; ++i){
                sem_post(arg->sem_a);
            }
            car = 1;
            *arg->num_a = *arg->num_a - 2;
            *arg->num_b = *arg->num_b - 2;
        }
        else{
            pthread_mutex_unlock(&lock_t);
            sem_wait(arg->sem_b);
        }
    }

    printf("Thread ID: %ld, Team: %c, I have found a spot in a car\n", pthread_self(), arg->team);
    pthread_barrier_wait(&barrier_t);
    
    if(car == 1) {
        //pthread_mutex_unlock(&lock_t);
        printf("Thread ID: %ld, Team: %c, I am the captain and driving the car\n", pthread_self(), arg->team);
        pthread_barrier_destroy(&barrier_t);
        pthread_barrier_init(&barrier_t, NULL, 4);
        pthread_mutex_unlock(&lock_t);
    }
    
    return NULL;
}


int main(int argc, const char * argv[]) {
    int num_a = atoi(argv[1]);
    int num_b = atoi(argv[2]);
    
    int total = num_b + num_a;
    
    if((num_a % 2 == 1 ) || (num_b % 2 == 1) || (total % 4 != 0)){
        printf("The main terminates\n");
        return 1;
    }
    
    srand(time(NULL));
    
    sem_t sem_a, sem_b;
    int num_A = 0, num_B = 0;
    
    pthread_mutex_init(&lock_t, NULL);
    pthread_barrier_init(&barrier_t, NULL, 4);
    
    pthread_t threads[total];
    struct thread_ t_args[total];
    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    
    for(int i = 0; i < total; ++i){
        t_args[i].team = 'C';
        t_args[i].sem_a = &sem_a;
        t_args[i].sem_b = &sem_b;
        t_args[i].num_a = &num_A;
        t_args[i].num_b = &num_B;
    }
    
    int rem_a = num_a;
    int rem_b = num_b;
    int c = total;
    
    while(c > 0){
        int rnd = rand() % total;
        if(t_args[rnd].team == 'C'){
            if(rem_a > 0){
                t_args[rnd].team = 'A';
                rem_a--;
                c--;
            }
            else if (rem_b > 0){
                t_args[rnd].team = 'B';
                rem_b--;
                c--;
            }
        }
    }
    
    for (int k = 0; k < total; k++) {
        pthread_create(&threads[k], NULL, pthread_, &t_args[k]);
    }
    
    for(int j = 0; j < total; ++j){
        pthread_join(threads[j], NULL);
    }

    pthread_barrier_destroy(&barrier_t);
    printf("The main terminates\n");
    sem_destroy(&sem_a);
    sem_destroy(&sem_b);

    return 0;
}
