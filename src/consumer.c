#include <stdio.h>
#include <stdlib.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <bufferhandler.h>
#include <sys/types.h>
#include <unistd.h>

struct Consumer
{
	pid_t PID;
	int times_mean;
	int op_mode;
	struct Message *buffer;
	sem_t *buffer_sem;
	struct shm_consumers *shmc;
	sem_t *shmc_sem;
} 
consumer;

void initializesConsumer(char *buffer_name, int random_times_mean, int op_mode);
void printMessageContent(struct Message *msg);

int main(int argc, char *argv[]) 
{
	if(argc != 4)
	{
		printf("\033[1;31m");
		printf("%s\n", "Error: you must write 3 arguments, buffer name, mean of the random times and operation mode");
		printf("\033[0m");
		exit(1);
	}

	initializesConsumer(argv[1], atoi(argv[2]), atoi(argv[3]));

	return 0;
}

void initializesConsumer(char *buffer_name, int random_times_mean, int operation_mode)
{
	if (op_mode != 0 || op_mode != 1)
	{
		printf("\033[1;31m");
		printf("%s\n", "Error: operation mode must be 0 (automatic) or 1 (manual)");
		printf("\033[0m");
		exit(1);
	}

	consumer.PID = getpid();
	consumer.times_mean = random_times_mean;
	consumer.op_mode = operation_mode;

	consumer.buffer = (struct Message *)readFromShareMemoryBlock(buffer_name);

	char *consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	consumer.buffer_sem = openSemaphore(consumers_sem_name);

	char *shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	consumer.shmc = (struct shm_consumers *)readShareMemoryBlock(shmc_name)

	char *shmc_sem_name = generateTagName(buffer_name, CONSUMER_SHM_SEM_TAG);
	consumer.semaphore = openSemaphore(shmc_sem_name);

	free(consumers_sem_name);
	free(shmc_name);
	free(shmc_sem_name);
}

void readMessage(int index)
{
	struct Message *msg = consumer.buffer + (index * sizeof(struct Message));

	// System suspend indicator
	if (msg->magic_number == -1)
	{
		// suspende el consumidor
	}

	printMessageContent(msg);

}

void printMessageContent(struct Message *msg)
{
	printf("Producer PID  : %i\n", msg->id);
	printf("Day           : %i\n", msg->date.day);
	printf("Month         : %i\n", msg->date.month);
	printf("Year          : %i\n", msg->date.year);
	printf("Hour          : %i\n", msg->time.hour);
	printf("Minutes       : %i\n", msg->time.minutes);
	printf("Seconds       : %i\n", msg->time.seconds);
	printf("Magic number  : %i\n", msg->magic_number);
}

void incrementConsumersTotal()
{
	consumer.shmc->consumers_total++;
}

void decrementConsumersTotal()
{
	consumer.shmc->consumers_total--;
}
