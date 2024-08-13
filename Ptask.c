#define _POSIX_C_SOURCE 200112L
#include <time.h>

#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "Ptask.h"
#include <semaphore.h>
#include <stdlib.h>
#include <math.h>
#include <sched.h>
#include <string.h>
#include <errno.h>


#define MAX_TASK 100
#define _GNU_SOURCE
#define MICRO 1000000
#define MILLI 1000
#define ACT 1

struct timespec ptask_t0;
int ptask_policy;
struct task_par tp[MAX_TASK];
//void *task(void *arg);


void time_copy(struct  timespec *td, struct timespec ts){
    td -> tv_sec = ts.tv_sec ;
    td -> tv_nsec = ts.tv_nsec;
}

void time_add_ms(struct timespec *t, int ms)
{
    t->tv_sec += ms/1000;
    t->tv_nsec += (ms%1000)*1000000;
    if (t->tv_nsec > 1000000000) {
    t->tv_nsec -= 1000000000;
    t->tv_sec += 1;
    }
}

int time_cmp(struct timespec t1,struct timespec t2)
    {
    if (t1.tv_sec > t2.tv_sec) return 1;
    if (t1.tv_sec < t2.tv_sec) return -1;
    if (t1.tv_nsec > t2.tv_nsec) return 1;
    if (t1.tv_nsec < t2.tv_nsec) return -1;
    return 0;
}


void ptask_init(int policy)
{
    int i;
    ptask_policy = policy;
    clock_gettime(CLOCK_MONOTONIC, &ptask_t0);
    // initialize activation semaphores
    for (i=0; i<MAX_TASK; i++)
    //sem_init vuole come primo argomento un
    // puntatore a una variabile di tipo semaforo
    sem_init(&tp[i].tsem, 0, 0);
}


long get_systime(int unit)
{
    struct timespec t;
    long tu, mul, div;
    switch (unit) {
    case MICRO: mul = 1000000; div = 1000; break;
    case MILLI: mul = 1000; div = 1000000; break;
    default: mul = 1000; div = 1000000; break;
    }
    clock_gettime(CLOCK_MONOTONIC, &t);
    tu = (t.tv_sec - ptask_t0.tv_sec)*mul;
    tu += (t.tv_nsec - ptask_t0.tv_nsec)/div;
    return tu;
}

/*periodic task implementation
void *task(void *arg)
{
    struct timespec t;
    int period = 100; // period in milliseconds 
    clock_gettime(CLOCK_MONOTONIC, &t);
    time_add_ms(&t, period);
    while (1) {
    //do useful work 
        clock_nanosleep(CLOCK_MONOTONIC,
        TIMER_ABSTIME, &t, NULL);
        time_add_ms(&t, period);
    }
}*/

int task_create( void* (*task) (void*), int i, int period, int drel, int prio, int aflag){
    pthread_attr_t myatt;
    struct sched_param mypar;
    int tret;
    if( i >= MAX_TASK) return -1;

    tp[i].arg = i;
    tp[i].period = period;
    tp[i].deadline = drel;
    tp[i].priority = prio;
    tp[i].dmiss = 0;
    pthread_attr_init(&myatt);
    pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
    //pthread_attr_setschedpolicy(&myatt, SCHED_RR);
     printf("Ptask aggrionato\n");
    mypar.sched_priority = tp[i].priority;
    pthread_attr_setschedparam(&myatt, &mypar);
    tret = pthread_create(&tp[i].id, &myatt, task,
    (void*)(&tp[i]));
    //il flag per attivare il task è avere la variabile aflag == 1
    if (aflag == ACT) task_activate(i);
     if (tret != 0) {
        printf("Errore nella creazione del thread: %s\n", strerror(tret));
    }

    return tret;
}

int get_task_index(void* arg){
	struct task_par *tpar;
		tpar = (struct task_par*)arg;
		return tpar-> arg;
}

void wait_for_activation(int i){
	struct timespec t;
	sem_wait(&tp[i].tsem);
	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp[i].at), t);
	time_copy(&(tp[i].dl), t);
	time_add_ms(&(tp[i].at), tp[i].period);
	time_add_ms(&(tp[i].dl), tp[i].deadline);
}

void task_activate(int i)
{
	sem_post(&tp[i].tsem);
}

int deadline_miss(int i)
{
	struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		if (time_cmp(now, tp[i].dl) > 0) {
			tp[i].dmiss++;
			return 1;
	}
	return 0;
}

void wait_for_period(int i)
{
	clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME, &(tp[i].at), NULL);

	time_add_ms(&(tp[i].at), tp[i].period);
	time_add_ms(&(tp[i].dl), tp[i].period);
}

void task_set_period(int i, int per)
{
	tp[i].period = per;
}

void task_set_deadline(int i, int dline)
{
	tp[i].deadline = dline;
}

int task_period(int i)
{
	return tp[i].period;
}

int task_deadline(int i)
{
	return tp[i].deadline;
}

int task_dmiss(int i)
{
	return tp[i].dmiss;
}

void task_atime(int i, struct timespec *at)
{
	at->tv_sec = tp[i].at.tv_sec;
	at->tv_nsec = tp[i].at.tv_nsec;
}

void task_adline(int i, struct timespec *dl)
{
	dl->tv_sec = tp[i].dl.tv_sec;
	dl->tv_nsec = tp[i].dl.tv_nsec;
}

void wait_for_task_end(int i)
{ 
	pthread_join(tp[i].id, NULL);
    printf("Task %d sta eseguendo\n", i);
}




