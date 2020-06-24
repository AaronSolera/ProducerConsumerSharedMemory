#include <shmhandler.h>

struct Consumer
{
	pid_t PID;
	int times_mean;
	int op_mode;
	int consumed_messages;
	int current_buffer_index;
	char * suspention_reason;
	double acum_waited_time;
	double acum_blocked_time;
	double user_time;
	struct Message *buffer;
	struct shm_consumers *shmc;
	struct shm_producers *shmp;
	sem_t  *shmc_sem;
	sem_t  *producers_buffer_sem;
	sem_t  *consumers_buffer_sem;
} 
consumer;

// This is value kills the consumer if it is TRUE
int kill = FALSE;
// Shared messages buffer size
int shm_block_size;
// This stores a random value
double r;

void initializesConsumer(char *buffer_name, int random_times_mean, int op_mode);
void printMessageContent(struct Message *msg);
void readMessage(int index);
void finalizeConsumer();

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

	// This main loop ends when kill variable is TRUE
	while(not(kill)) {
		// Stopping process with random exponential behavior values
		sleep(exp(consumer.times_mean));
		// Decrement global consumer bufer semaphore
		sem_wait(consumer.shmc_sem);
		// Storing consumer writting buffer index value for keeping untouchable for other process
		consumer.current_buffer_index = consumer.shmc->buffer_index;
		// This lines increments index to be written in messages buffer.
		consumer.shmc->buffer_index++;
		consumer.shmc->buffer_index = consumer.shmc->buffer_index % (shm_block_size / sizeof(struct Message));
		// Incrementing global consumer bufer semaphore
		sem_post(consumer.shmc_sem);
		// Decrementing consumer messages buffer semaphore for blocking one index from that buffer
		sem_wait(consumer.consumers_buffer_sem);
		// Reading a new message in the shared buffer
		readMessage(consumer.current_buffer_index);
		// Incrementing consumed messages number just for statistics
		consumer.consumed_messages++;
		// Incrementing consumer semaphore to access the shared messages buffer
		sem_post(consumer.producers_buffer_sem);
	}
	// Printing in terminal a finalized producer alarm and some statistics
	printf("\033[1;33m---------------------------------------------------\n");
	printf("|The cosumer whose id is %-5i has been finalized |\n", consumer.PID);
	printf("|-------------------------------------------------|\n");
	printf("|\033[0;33mConsumed messages %-10d                     \033[1;33m|\n", consumer.consumed_messages);
	printf("|\033[0;33mWaited time       %-10f                     \033[1;33m|\n", consumer.acum_waited_time);
	printf("|\033[0;33mBlocked time      %-10f                     \033[1;33m|\n", consumer.acum_blocked_time);
	printf("|\033[0;33mUser time         %-10f                     \033[1;33m|\n", consumer.user_time);
	printf("---------------------------------------------------\033[0m\n");
	// Printing suspention reason
	printf("\033[1;31mSuspention reason: %s\033[0m\n", consumer.suspention_reason);
	return 0;
}

void initializesConsumer(char *buffer_name, int random_times_mean, int operation_mode)
{
	if (operation_mode != 0 && operation_mode != 1)
	{
		printf("\033[1;31m");
		printf("%s\n", "Error: operation mode must be 0 (automatic) or 1 (manual)");
		printf("\033[0m");
		exit(1);
	}

	consumer.PID = getpid();
	consumer.times_mean = random_times_mean;
	consumer.op_mode = operation_mode;

	consumer.buffer = (struct Message *) mapShareMemoryBlock(buffer_name);

	// Opening consumer buffer access semaphore and storing its file descriptor
	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	consumer.producers_buffer_sem = openSemaphore(producers_sem_name);
	// Opening consumer buffer access semaphore and storing its file descriptor
	char *consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	consumer.consumers_buffer_sem = openSemaphore(consumers_sem_name);

	char *shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	consumer.shmc = (struct shm_consumers *) mapShareMemoryBlock(shmc_name);
	// Mapping shared consumer global variables buffer and storing its memory address
	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	consumer.shmp = (struct shm_producers *) mapShareMemoryBlock(shmp_name);

	char *shmc_sem_name = generateTagName(buffer_name, CONSUMER_SHM_SEM_TAG);
	consumer.shmc_sem = openSemaphore(shmc_sem_name);
	// Decrementing global consumer bufer semaphore
	sem_wait(consumer.shmc_sem);
	// Incrementing total consumers value
	consumer.shmc->consumers_total++;
	// Incrementing global consumer bufer semaphore
	sem_post(consumer.shmc_sem);
	// Storing shared messages buffer size for writing index computing
	shm_block_size = getShareMemoryBlockSize(buffer_name);
	// Setting some timing and counting statatistic values to 0
	consumer.consumed_messages = 0;
	consumer.acum_waited_time = 0;
	consumer.acum_blocked_time = 0;
	consumer.user_time = 0;

	free(consumers_sem_name);
	free(producers_sem_name);
	free(shmc_name);
	free(shmp_name);
	free(shmc_sem_name);
}

void readMessage(int index)
{
	struct Message *msg = consumer.buffer + index;

	// System suspend indicator
	if (msg->magic_number == -1)
	{
		finalizeConsumer();
		consumer.suspention_reason = "Finalized by finalizer process.";
	} else if (msg->magic_number == consumer.PID % 6)
	{
		finalizeConsumer();
		consumer.suspention_reason = "Read key is equal to id % 6.";
	} else {
		printf("\033[1;32m----------------------------------------------\n");
		printf("| A message was read in shared memory block  |\n");
		printf("|--------------------------------------------|\n");
		printf("|\033[0;32mBuffer index     %-10i                 \033[1;32m|\n", consumer.current_buffer_index);
		printf("|\033[0;32mConsumers alive  %-10i                 \033[1;32m|\n", consumer.shmc->consumers_total);
		printf("|\033[0;32mProducers alive  %-10i                 \033[1;32m|\n", consumer.shmp->producers_total);
		printf("|--------------------------------------------|\n");
		printMessageContent(msg);
		printf("----------------------------------------------\033[0m\n");
	}
}

void printMessageContent(struct Message *msg)
{
	printf("|\033[0;32mProducer PID  : %-10i                  \033[1;32m|\n", msg->id);
	printf("|\033[0;32mDay           : %-10i                  \033[1;32m|\n", msg->date.day);
	printf("|\033[0;32mMonth         : %-10i                  \033[1;32m|\n", msg->date.month);
	printf("|\033[0;32mYear          : %-10i                  \033[1;32m|\n", msg->date.year);
	printf("|\033[0;32mHour          : %-10i                  \033[1;32m|\n", msg->time.hour);
	printf("|\033[0;32mMinutes       : %-10i                  \033[1;32m|\n", msg->time.minutes);
	printf("|\033[0;32mSeconds       : %-10i                  \033[1;32m|\n", msg->time.seconds);
	printf("|\033[0;32mMagic number  : %-10i                  \033[1;32m|\n", msg->magic_number);
}

void finalizeConsumer() {
	// Decrementing global consumer bufer semaphore
	sem_wait(consumer.shmc_sem);
	// Incrementing total consumers value
	consumer.shmc->consumers_total--;
	// Incrementing global consumer bufer semaphore
	sem_post(consumer.shmc_sem);
	kill = TRUE;
}