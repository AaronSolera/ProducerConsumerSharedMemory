#include <stdio.h>
#include <stdlib.h>
#include <bufferhandler.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void initializesProducer(char *buffer_name, int random_times_mean);
void writeNewMessage(struct Message *buffer, int index),

struct Producer
{
	pid_t PID;
	int times_mean;
	struct Message *buffer;
	sem_t *buffer_sem;
	struct shm_producers *shmp;
	sem_t *shmp_sem;
} 
producer;

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



	createShrareMemoryBlock(shm_name, 8);
	writeShrareMemoryBlock(shm_name, "HolaCoca", 0);
	char * msg = readShrareMemoryBlock(shm_name, 8, 0);
	printf("%s\n", msg);
	writeShrareMemoryBlock(shm_name, "Mota", 4);
	msg = readShrareMemoryBlock(shm_name, 8, 0);
	printf("%s\n", msg);
	free(msg);
	deleteShrareMemoryBlock(shm_name); 

	return 0;
}


void initializesProducer(char *buffer_name, int random_times_mean)
{
	producer.PID = getpid();
	producer.times_mean = random_times_mean;

	producer.buffer = (struct Message *)readFromShareMemoryBlock(buffer_name);

	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	producer.buffer_sem = openSemaphore(producers_sem_name);

	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	producer.shmp = (struct shm_producers *)readShareMemoryBlock(shmp_name);

	char *shmp_sem_name = generateTagName(buffer_name, PRODUCER_SHM_SEM_TAG);
	producer.shmp_sem = openSemaphore(shmp_sem_name);

	free(producers_sem_name);
	free(shmp_name);
	free(shmp_sem_name);
}


void writeNewMessage(struct Message *buffer, int index)
{
	time_t raw_time;
 	time(&raw_time);
  	struct tm *time_info = localtime(&raw_time);
  	srand(time(NULL));

	struct Message new_msg;	
	new_msg.id = getpid();
	new_msg.date.day = time_info->tm_mday;
	new_msg.date.month = time_info->tm_mon;
	new_msg.date.year = time_info->tm_year + 1900;
	new_msg.time.hour = time_info->tm_hour;
	new_msg.time.minutes = time_info->tm_min;
	new_msg.time.seconds = time_info->tm_sec;
	new_msg.magic_number = rand() % (MAX_MAGIC_NUMBER + 1);

	// Writes the new message to the corresponding buffer index
	memcpy(buffer + index * sizeof(struct Mesage), new_msg, sizeof(struct Mesage));
}
