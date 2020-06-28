#include <shmhandler.h>

void initializesFinalizer(char *buffer_name);
void finalizeProducer();
void writeNewMessage(int index);


// This structure stores all podrucer relevant values
struct Finalizer {
	pid_t PID;
	int    produced_messages;
	int    current_buffer_index;
	double waited_time;
	double sem_blocked_time;
	long int kernel_time;
	struct Message *buffer;
	struct shm_producers *shmp;
	struct shm_consumers *shmc;
	sem_t  *producer_buffer_sem;
	sem_t  *consumer_buffer_sem;
	sem_t  *shmp_sem;
	sem_t  *shmc_sem; 
} finalizer;

// Declaring semaphore names
char *producers_sem_name;
char *consumers_sem_name;
char *shmp_sem_name;
char *shmc_sem_name;
// Declaring shared gobal memory variable names
char *shmp_name;
char *shmc_name;
// This struct is used for taking user space time
struct rusage ktime;
// This is value kills the producer if it is TRUE
int kill = FALSE;
// Shared messages buffer size
int shm_block_size;
// This stores a random value
double r;
int total_consumers;
int main(int argc, char *argv[]) {
	// Ckecking if executable file gets just 2 arguments
	if(argc != 2) {
		printf("%s\n", "\033[1;31mError: you must write 1 arguments, only the buffer name\033[0m");
		exit(1);
	}
    // Initializing all needed values 
	initializesFinalizer(argv[1]);
	// Decrement global producer bufer semaphore
	sem_wait(finalizer.shmp_sem);
	finalizer.shmp->buffer_isActive = 0;
	// Incrementing global producer bufer semaphorefinalizer
	sem_post(finalizer.shmp_sem);
	// For the number of producers alive
	for (int i = 0; i <= finalizer.shmp->producers_total; ++i)
	{
		// Incrementing global producer bufer semaphorefinalizer
		sem_post(finalizer.producer_buffer_sem);
	}
	// For the number of consumers alive
	while(finalizer.shmc->consumers_total > 0)
	{
		// Decrement global producer bufer semaphore
		sem_wait(finalizer.shmp_sem);
		// Storing producer writting buffer index value for keeping untouchable for other process
		finalizer.current_buffer_index = finalizer.shmp->buffer_index;
		// This lines increments index to be written in messages buffer.
		finalizer.shmp->buffer_index++;
		finalizer.shmp->buffer_index = finalizer.shmp->buffer_index % (shm_block_size / sizeof(struct Message));
		// Incrementing global producer bufer semaphorefinalizer
		sem_post(finalizer.shmp_sem);
		// Decrementing producer messages buffer semaphore for blocking one index from that buffer
		sem_wait(finalizer.producer_buffer_sem);
		// Writing a new message into the shared buffer
		writeNewMessage(finalizer.current_buffer_index);
		// Incrementing
		sem_post(finalizer.consumer_buffer_sem);
	}

	// Printing in terminal a written message alarm and some statistics
	printf("\033[1;32m-------------------------------------\n");
	printf("|Finalizer statistics               |\n");
	printf("|-----------------------------------|\n");
	printf("|\033[0;32mTotal messages         %-10i  \033[1;32m|\n", finalizer.shmp->produced_messages);
	printf("|\033[0;32mTotal consumers        %-10i  \033[1;32m|\n", finalizer.shmc->accum_consumers);
	printf("|\033[0;32mTotal producers        %-10i  \033[1;32m|\n", finalizer.shmp->accum_producers);
	printf("|\033[0;33mKey deleted consumers  %-10d  \033[1;33m|\n", finalizer.shmc->key_deleted);
	printf("|\033[0;33mTotal waited time      %-10f  \033[1;33m|\n", finalizer.shmc->total_waited_time + finalizer.shmp->total_waited_time);
	printf("|\033[0;33mTotal blocked time     %-10f  \033[1;33m|\n", finalizer.shmc->total_blocked_time + finalizer.shmp->total_blocked_time);
	printf("|\033[0;33mTotal user time (us)   %-10i  \033[1;33m|\n", finalizer.shmc->total_user_time);
	printf("|\033[0;33mTotal kernel time (us) %-10i  \033[1;33m|\n", finalizer.shmp->total_kernel_time);
	printf("-------------------------------------\033[0m\n");
	// Setting semaphores free by name 
	sem_unlink(producers_sem_name);
	sem_unlink(consumers_sem_name);
	sem_unlink(shmc_sem_name);
	sem_unlink(shmp_sem_name);
	// Setting shared memory blocks free by name
	deleteShareMemoryBlock(shmp_name);
	deleteShareMemoryBlock(shmc_name);
	deleteShareMemoryBlock(argv[1]);
	// Setting free used string allocated memory 
	free(producers_sem_name);
	free(consumers_sem_name);
	free(shmc_sem_name);
	free(shmp_sem_name);
	free(shmp_name);
	free(shmc_name);

	return 0;
}

void initializesFinalizer(char *buffer_name) {
	// Mapping messages shared buffer address
	finalizer.buffer = (struct Message *) mapShareMemoryBlock(buffer_name);
	// Opening producer buffer access semaphore and storing its file descriptor
	producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	finalizer.producer_buffer_sem = openSemaphore(producers_sem_name);
	// Opening consumer buffer access semaphore and storing its file descriptor
	consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	finalizer.consumer_buffer_sem = openSemaphore(consumers_sem_name);
	// Opening shared consumer global variables buffer semaphore and storing its file descriptor
	shmc_sem_name = generateTagName(buffer_name, CONSUMER_SHM_SEM_TAG);
	finalizer.shmc_sem = openSemaphore(shmc_sem_name);
	// Opening shared producer global variables buffer semaphore and storing its file descriptor
	shmp_sem_name = generateTagName(buffer_name, PRODUCER_SHM_SEM_TAG);
	finalizer.shmp_sem = openSemaphore(shmp_sem_name);
	// Mapping shared producer global variables buffer and storing its memory address
	shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	finalizer.shmp = (struct shm_producers *) mapShareMemoryBlock(shmp_name);
	// Mapping shared consumer global variables buffer and storing its memory address
	shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	finalizer.shmc = (struct shm_consumers *) mapShareMemoryBlock(shmc_name);
	// Decrementing global producer bufer semaphore
	// Storing shared messages buffer size for writing index computing
	shm_block_size = getShareMemoryBlockSize(buffer_name);
	// Setting some timing and counting statatistic values to 0
	finalizer.produced_messages = 0;
	finalizer.waited_time = 0;
	finalizer.sem_blocked_time = 0;
	finalizer.kernel_time = 0;
}

void writeNewMessage(int index) {
	// Getting current time info
	time_t raw_time;
 	time(&raw_time);
  	struct tm *time_info = localtime(&raw_time);
  	srand(time(NULL));
  	// Creating and filling message struct with its relevant information
	struct Message new_msg;	
	new_msg.id = finalizer.PID;
	new_msg.date.day = time_info->tm_mday;
	new_msg.date.month = time_info->tm_mon + 1;
	new_msg.date.year = time_info->tm_year + 1900;
	new_msg.time.hour = time_info->tm_hour;
	new_msg.time.minutes = time_info->tm_min;
	new_msg.time.seconds = time_info->tm_sec;
	new_msg.magic_number = -1;
	// Writing the whole structure into shared messages buffer memory
	writeInShareMemoryBlock(finalizer.buffer, &new_msg, sizeof(struct Message), index);
}

