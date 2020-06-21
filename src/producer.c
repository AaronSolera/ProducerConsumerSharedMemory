#include <stdio.h>
#include <stdlib.h>
#include <shmhandler.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "global.h"

void initializesProducer(char *buffer_name, double random_times_mean);
void writeNewMessage(int index);
double exp(double x);

struct Producer
{
	pid_t PID;
	double times_mean;
	int produced_messages;
	double waited_time;
	double sem_blocked_time;
	double kernel_time;
	struct Message *buffer;
	sem_t *buffer_sem;
	struct shm_producers *shmp;
	sem_t *shmp_sem;
} producer;

int main(int argc, char *argv[]) 
{
	if(argc != 3)
	{
		printf("\033[1;31m");
		printf("%s\n", "Error: you must write 2 arguments, buffer name and mean of the random times");
		printf("\033[0m");
		exit(1);
	}
    
	initializesProducer(argv[1], atoi(argv[2]));
	writeNewMessage(0);

	/*
	while(TRUE){
		sem_wait(producer.shmp_sem);
		sem_wait(producer.buffer_sem);
		printf("Running");
		//writeNewMessage(0);
		//writeInShareMemoryBlock(argv[1], &new_msg, 0);
		//sem_wait(producer.buffer_sem);
		//sem_wait(producer.shmp_sem);
	}*/

	return 0;
}


void initializesProducer(char *buffer_name, double random_times_mean)
{
	producer.PID = getpid();
	producer.times_mean = exp(random_times_mean);

	//producer.buffer = (struct Message *) readFromShareMemoryBlock(buffer_name);
	producer.buffer = (struct Message *) mapShareMemoryBlock(buffer_name);

	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	producer.buffer_sem = openSemaphore(producers_sem_name);

	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	//producer.shmp = (struct shm_producers *) readFromShareMemoryBlock(shmp_name);
	producer.shmp = (struct shm_producers *) mapShareMemoryBlock(shmp_name);

	char *shmp_sem_name = generateTagName(buffer_name, PRODUCER_SHM_SEM_TAG);
	producer.shmp_sem = openSemaphore(shmp_sem_name);

	free(producers_sem_name);
	free(shmp_name);
	free(shmp_sem_name);
}


void writeNewMessage(int index)
{
	time_t raw_time;
 	time(&raw_time);
  	struct tm *time_info = localtime(&raw_time);
  	srand(time(NULL));

	struct Message new_msg;	
	new_msg.id = producer.PID;
	new_msg.date.day = time_info->tm_mday;
	new_msg.date.month = time_info->tm_mon;
	new_msg.date.year = time_info->tm_year + 1900;
	new_msg.time.hour = time_info->tm_hour;
	new_msg.time.minutes = time_info->tm_min;
	new_msg.time.seconds = time_info->tm_sec;
	new_msg.magic_number = rand() % (MAX_MAGIC_NUMBER + 1);

	writeInShareMemoryBlock(producer.buffer, &new_msg, sizeof(struct Message), index);
}

double exp(double x){
	double r;
	r = rand() / (RAND_MAX + 1.0);
	return -log(1 - r) / x;
}
