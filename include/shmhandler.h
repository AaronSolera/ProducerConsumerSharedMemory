#ifndef _SHMHANDLER_H
#define _SHMHANDLER_H  

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#define PRODUCER_SEM_TAG		"_producers_sem"
#define CONSUMER_SEM_TAG		"_consumers_sem"
#define PRODUCER_SHM_TAG  		"_producers_shm"
#define CONSUMER_SHM_TAG  		"_consumers_shm"
#define PRODUCER_SHM_SEM_TAG	"_consumers_shm_sem"
#define CONSUMER_SHM_SEM_TAG	"_producers_shm_sem"
#define MAX_MAGIC_NUMBER		6
#define TRUE 					1
#define FALSE 					0

struct Date 
{
	int day;
	int month;
	int year;
};

struct Time 
{
	int hour;
	int minutes;
	int seconds;
};

struct Message 
{
	pid_t id;
	struct Date date;
	struct Time time;
	int magic_number;
};

struct shm_producers 
{
	int producers_total;
	int buffer_index;
	int buffer_isActive;
	int produced_messages;
	int accum_producers;
	double total_waited_time;
	double total_blocked_time;
	int total_kernel_time;
};

struct shm_consumers 
{
	int consumers_total;
	int buffer_index;
	int accum_consumers;
	int key_deleted;
	double total_waited_time;
	double total_blocked_time;
	int total_user_time;
};

void createShareMemoryBlock(char * buffer_name, int size);
void * mapShareMemoryBlock(char * buffer_name);
void writeInShareMemoryBlock(void * ptr, void * data, int size, int offset);
void deleteShareMemoryBlock(char * buffer_name);
int getShareMemoryBlockSize(char * buffer_name);

char * generateTagName(char *name, const char *tag);
sem_t * openSemaphore(char *name);
int not(int boolean);

#endif