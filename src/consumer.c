#include <shmhandler.h>

#define ENTER_ASCII_CODE 10

struct Consumer
{
	pid_t PID;
	int times_mean;
	int op_mode;
	int consumed_messages;
	int current_buffer_index;
	char * suspention_reason;
	double waited_time;
	double blocked_time;
	long int user_stime;
	struct Message *buffer;
	struct shm_consumers *shmc;
	struct shm_producers *shmp;
	sem_t  *shmc_sem;
	sem_t  *producers_buffer_sem;
	sem_t  *consumers_buffer_sem;
} 
consumer;

// This struct is used for taking user space time
struct rusage utime;
// This is value kills the consumer if it is TRUE
int kill = FALSE;
// Boolean flag to mode setting
int key_mode = FALSE;
// Shared messages buffer size
int shm_block_size;
// This stores a random value
double r;
// This variables stores 
clock_t waited_time_begin, waited_time_end;
clock_t blocked_time_begin, blocked_time_end;

void initializesConsumer(char *buffer_name, int random_times_mean, int op_mode);
void printMessageContent(struct Message *msg);
void readMessage(int index);
void finalizeConsumer();
double posison(double lambda);


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
		// Checking the set mode
		if(key_mode){
			// Waiting for entering key code
			printf("\033[0;36mPress \033[1;33mEnter\033[0;36m to consume messages from producers...\033[0m\n");
			// Saving starting waited time 
			waited_time_begin = clock();
			while(getchar() != ENTER_ASCII_CODE);
			// Saving ending waited time 
			waited_time_end = clock();
			// Storing defference between end and start time into waited time
			consumer.waited_time += (double)(waited_time_end - waited_time_begin) / CLOCKS_PER_SEC;
		}else{
			// Saving starting waited time 
			waited_time_begin = clock();
			// Stopping process with random poisson behavior values
			sleep(posison(consumer.times_mean));
			// Saving ending waited time 
			waited_time_end = clock();
			// Storing defference between end and start time into waited time
			consumer.waited_time += (double)(waited_time_end - waited_time_begin) / CLOCKS_PER_SEC;
		}
		// Saving starting blocked time 
		blocked_time_begin = clock();
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
		// Saving ending waited time 
		blocked_time_end = clock();
		// Storing defference between end and start time into waited time
		consumer.blocked_time += (double)(blocked_time_end - blocked_time_begin) / CLOCKS_PER_SEC;
		// Reading a new message in the shared buffer
		readMessage(consumer.current_buffer_index);
	}

	// Printing in terminal a finalized producer alarm and some statistics
	printf("\033[1;33m---------------------------------------------------\n");
	printf("|The cosumer whose id is %-5i has been finalized |\n", consumer.PID);
	printf("|-------------------------------------------------|\n");
	printf("|\033[0;33mConsumed messages %-10d                     \033[1;33m|\n", consumer.consumed_messages);
	printf("|\033[0;33mWaited time (s)   %-10f                     \033[1;33m|\n", consumer.waited_time);
	printf("|\033[0;33mBlocked time (s)  %-10f                     \033[1;33m|\n", consumer.blocked_time);
	printf("|\033[0;33mUser time (us)    %-10li                     \033[1;33m|\n", consumer.user_stime);
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
	// Checking and setting the operation mode
	if(operation_mode == 1){
		key_mode = TRUE;
	}

	consumer.PID = getpid();
	consumer.times_mean = random_times_mean;
	consumer.op_mode = operation_mode;
	// Mapping the shared messages buffer block memory
	consumer.buffer = (struct Message *) mapShareMemoryBlock(buffer_name);
	// Opening consumer buffer access semaphore and storing its file descriptor
	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	consumer.producers_buffer_sem = openSemaphore(producers_sem_name);
	// Opening consumer buffer access semaphore and storing its file descriptor
	char *consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	consumer.consumers_buffer_sem = openSemaphore(consumers_sem_name);
	// Mapping shared consumer global variables buffer and storing its memory address
	char *shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	consumer.shmc = (struct shm_consumers *) mapShareMemoryBlock(shmc_name);
	// Mapping shared producer global variables buffer and storing its memory address
	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	consumer.shmp = (struct shm_producers *) mapShareMemoryBlock(shmp_name);

	char *shmc_sem_name = generateTagName(buffer_name, CONSUMER_SHM_SEM_TAG);
	consumer.shmc_sem = openSemaphore(shmc_sem_name);
	// Decrementing global consumer bufer semaphore
	sem_wait(consumer.shmc_sem);
	// Incrementing total consumers value
	consumer.shmc->consumers_total++;
	consumer.shmc->accum_consumers++;
	// Incrementing global consumer bufer semaphore
	sem_post(consumer.shmc_sem);
	// Storing shared messages buffer size for writing index computing
	shm_block_size = getShareMemoryBlockSize(buffer_name);
	// Setting some timing and counting statatistic values to 0
	consumer.consumed_messages = 0;
	consumer.waited_time = 0;
	consumer.blocked_time = 0;
	// Getting process statistic struct
	getrusage(RUSAGE_SELF, &utime);
	// Setting free used string allocated memory 
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
		// Decrementing global consumer bufer semaphore
		sem_wait(consumer.shmc_sem);
		consumer.shmc->key_deleted++;
		// Incrementing global consumer bufer semaphore
		sem_post(consumer.shmc_sem);
		consumer.suspention_reason = "Read key is equal to id % 6.";
	}
	// Incrementing consumed messages number just for statistics
	consumer.consumed_messages++;
	// Incrementing producer semaphore to access the shared messages buffer
	sem_post(consumer.producers_buffer_sem);

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

void printMessageContent(struct Message *msg)
{
	printf("|\033[0;32mProducer PID     %-10i                 \033[1;32m|\n", msg->id);
	printf("|\033[0;32mDay              %-10i                 \033[1;32m|\n", msg->date.day);
	printf("|\033[0;32mMonth            %-10i                 \033[1;32m|\n", msg->date.month);
	printf("|\033[0;32mYear             %-10i                 \033[1;32m|\n", msg->date.year);
	printf("|\033[0;32mHour             %-10i                 \033[1;32m|\n", msg->time.hour);
	printf("|\033[0;32mMinutes          %-10i                 \033[1;32m|\n", msg->time.minutes);
	printf("|\033[0;32mSeconds          %-10i                 \033[1;32m|\n", msg->time.seconds);
	printf("|\033[0;32mMagic number     %-10i                 \033[1;32m|\n", msg->magic_number);
}

void finalizeConsumer() {
	// Setting time in user space
	consumer.user_stime = (long int) utime.ru_utime.tv_usec;
	// Decrementing global consumer bufer semaphore
	sem_wait(consumer.shmc_sem);
	// Incrementing total consumers value
	consumer.shmc->consumers_total--;
	consumer.shmc->total_waited_time += consumer.waited_time;
	consumer.shmc->total_blocked_time += consumer.blocked_time;
	consumer.shmc->total_user_time += consumer.user_stime;
	// Incrementing global consumer bufer semaphore
	sem_post(consumer.shmc_sem);
	// Setting the killing boolean variable
	kill = TRUE;
}

double posison(double lambda) {
	r = rand() / (RAND_MAX + 1.0);
	return log(log(r / lambda));
}