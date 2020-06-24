#include <shmhandler.h>

void initializesProducer(char *buffer_name, double random_times_mean);
void finalizeProducer();
void writeNewMessage(int index);
double exp(double x);

// This structure stores all podrucer relevant values
struct Producer {
	pid_t  PID;
	int    produced_messages;
	int    current_buffer_index;
	double times_mean;
	double waited_time;
	double sem_blocked_time;
	double kernel_time;
	struct Message *buffer;
	struct shm_producers *shmp;
	struct shm_consumers *shmc;
	sem_t  *producer_buffer_sem;
	sem_t  *consumer_buffer_sem;
	sem_t  *shmp_sem;
} producer;

// This is value kills the producer if it is TRUE
int kill = FALSE;
// Shared messages buffer size
int shm_block_size;
// This stores a random value
double r;

int main(int argc, char *argv[]) {

	// Ckecking if executable file gets just 2 arguments
	if(argc != 3) {
		printf("%s\n", "\033[1;31mError: you must write 2 arguments, buffer name and mean of the random times\033[0m");
		exit(1);
	}
    // Initializing all needed values 
	initializesProducer(argv[1], atoi(argv[2]));

	//int sem_value; <Debuging>

	// This main loop ends when kill variable is TRUE
	while(not(kill)) {
		// Stopping process with random exponential behavior values
		sleep(exp(producer.times_mean));
		//sem_getvalue(producer.buffer_sem, &sem_value); <Debuging>
		//printf("%i\n", sem_value); <Debuging>
		// If keeping alive producer global varible is TRUE, the producer writes a message into the buffer. If not, finalize producer
		if(producer.shmp->buffer_isActive) {
			// Decrement global producer bufer semaphore
			sem_wait(producer.shmp_sem);
			// Storing producer writting buffer index value for keeping untouchable for other process
			producer.current_buffer_index = producer.shmp->buffer_index;
			// This lines increments index to be written in messages buffer.
			producer.shmp->buffer_index++;
			producer.shmp->buffer_index = producer.shmp->buffer_index % (shm_block_size / sizeof(struct Message));
			// Incrementing global producer bufer semaphore
			sem_post(producer.shmp_sem);
			// Decrementing producer messages buffer semaphore for blocking one index from that buffer
			sem_wait(producer.producer_buffer_sem);
			// Writing a new message into the shared buffer
			writeNewMessage(producer.current_buffer_index);
			// Incrementing produced messages number just for statistics
			producer.produced_messages++;
			// Incrementing
			sem_post(producer.consumer_buffer_sem);
			// Printing in terminal a written message alarm and some statistics
			printf("\033[1;32m----------------------------------------------\n");
			printf("|A message was written in shared memory block|\n");
			printf("|--------------------------------------------|\n");
			printf("|\033[0;32mBuffer index     %-10i                 \033[1;32m|\n", producer.current_buffer_index);
			printf("|\033[0;32mConsumers alive  %-10i                 \033[1;32m|\n", producer.shmc->consumers_total);
			printf("|\033[0;32mProducers alive  %-10i                 \033[1;32m|\n", producer.shmp->producers_total);
			printf("----------------------------------------------\033[0m\n");
		} else {
			// Calling finalize producer function
			finalizeProducer();
		}
		
	}
	// Printing in terminal a finalized producer alarm and some statistics
	printf("\033[1;33m---------------------------------------------------\n");
	printf("|The producer whose id is %-5i has been finalized|\n", producer.PID);
	printf("|-------------------------------------------------|\n");
	printf("|\033[0;33mProduced messages %-10d                     \033[1;33m|\n", producer.produced_messages);
	printf("|\033[0;33mWaited time       %-10f                     \033[1;33m|\n", producer.waited_time);
	printf("|\033[0;33mBlocked time      %-10f                     \033[1;33m|\n", producer.sem_blocked_time);
	printf("|\033[0;33mKernel time       %-10f                     \033[1;33m|\n", producer.kernel_time);
	printf("---------------------------------------------------\033[0m\n");

	return 0;
}

void initializesProducer(char *buffer_name, double random_times_mean) {
	// Setting producer id with the process id given by the kernel
	producer.PID = getpid();
	// Getting generating time mean given by the executable argument and storing it 
	producer.times_mean = random_times_mean;
	// Mapping messages shared buffer address
	producer.buffer = (struct Message *) mapShareMemoryBlock(buffer_name);
	// Opening producer buffer access semaphore and storing its file descriptor
	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	producer.producer_buffer_sem = openSemaphore(producers_sem_name);
	// Opening consumer buffer access semaphore and storing its file descriptor
	char *consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	producer.consumer_buffer_sem = openSemaphore(consumers_sem_name);
	// Mapping shared producer global variables buffer and storing its memory address
	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	producer.shmp = (struct shm_producers *) mapShareMemoryBlock(shmp_name);
	// Mapping shared consumer global variables buffer and storing its memory address
	char *shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	producer.shmc = (struct shm_consumers *) mapShareMemoryBlock(shmc_name);
	// Opening shared producer global variables buffer semaphore and storing its file descriptor
	char *shmp_sem_name = generateTagName(buffer_name, PRODUCER_SHM_SEM_TAG);
	producer.shmp_sem = openSemaphore(shmp_sem_name);
	// Incrementing total producers value
	producer.shmp->producers_total++;
	// Storing shared messages buffer size for writing index computing
	shm_block_size = getShareMemoryBlockSize(buffer_name);
	// Setting some timing and counting statatistic values to 0
	producer.produced_messages = 0;
	producer.waited_time = 0;
	producer.sem_blocked_time = 0;
	producer.kernel_time = 0;
	// Setting free used string allocated memory 
	free(producers_sem_name);
	free(shmp_name);
	free(shmc_name);
	free(shmp_sem_name);
}

void finalizeProducer() {
	// Decrementing total producers value
	producer.shmp->producers_total--;
	// Setting kill value to TRUE. This is going to end the process, finalizing the main loop decribed before
	kill = TRUE;
}

void writeNewMessage(int index) {
	// Getting current time info
	time_t raw_time;
 	time(&raw_time);
  	struct tm *time_info = localtime(&raw_time);
  	srand(time(NULL));
  	// Creating and filling message struct with its relevant information
	struct Message new_msg;	
	new_msg.id = producer.PID;
	new_msg.date.day = time_info->tm_mday;
	new_msg.date.month = time_info->tm_mon;
	new_msg.date.year = time_info->tm_year + 1900;
	new_msg.time.hour = time_info->tm_hour;
	new_msg.time.minutes = time_info->tm_min;
	new_msg.time.seconds = time_info->tm_sec;
	new_msg.magic_number = rand() % (MAX_MAGIC_NUMBER + 1);
	// Writing the whole structure into shared messages buffer memory
	writeInShareMemoryBlock(producer.buffer, &new_msg, sizeof(struct Message), index);
}

double exp(double x) {
	r = rand() / (RAND_MAX + 1.0);
	return -log(1 - r) / x;
}
