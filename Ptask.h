#include <stdio.h>
#include <pthread.h>
#include <time.h>
//#include <linux/time.h>
#include <semaphore.h>
#define _GNU_SOURCE
#include <sched.h>

//External global variables




// functions prototyes

void time_copy(struct  timespec *td, struct timespec ts);
void time_add_ms(struct timespec *t, int ms);
int time_cmp(struct timespec t1,struct timespec t2);
void ptask_init(int policy);
long get_systime(int unit);

int task_create( void* (*task) (void*), int i, int period, int drel, int prio, int alfag);
int get_task_index(void* arg);
void wait_for_activation(int i);
void task_activate(int i);
int deadline_miss(int i);
void wait_for_period(int i);
void task_set_period(int i, int per);
void task_set_deadline(int i, int dline);
int task_period(int i);
int task_deadline(int i);
int task_dmiss(int i);
void task_atime(int i, struct timespec *at);
void task_adline(int i, struct timespec *dl);
void wait_for_task_end(int i);

//structure declaration

struct task_par {
    int arg;
    long wcet;
    int period;
    int deadline;
    int priority;
    int dmiss;
    struct timespec at;
    struct timespec dl;
    pthread_t id;
    sem_t tsem;
    
};

