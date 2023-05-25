#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 1
#define STATE_CHANGE_PROB 10

#define ROOT 0

extern int rank;
extern int size;
extern int ackCount;
extern pthread_t threadKom;
extern int l_clock; // zegar
extern sem_t l_clock_sem; // semafor, broni dostępu do zmiennej zegara

#ifdef DEBUG
#define debug(FORMAT,...) printf("%c[%d;%dm [t:%d] [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, l_clock, rank, ##__VA_ARGS__, 27,0,37);
#else
#define debug(...) ;
#endif

#define println(FORMAT,...) printf("%c[%d;%dm [t:%d] [%d]: " FORMAT "%c[%d;%dm\n",  27, (1+(rank/7))%2, 31+(6+rank)%7, l_clock, rank, ##__VA_ARGS__, 27,0,37);


#endif
