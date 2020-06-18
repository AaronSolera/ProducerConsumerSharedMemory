#include <stdio.h>
#include <stdlib.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <bufferhandler.h>

char * shm_name;
int shm_size;

struct shm_block {
	int consumers_total;
	int producers_total;
	sem_t * consumer_semaphore;
	sem_t * producer_semaphore;
} memory;

int main(int argc, char *argv[]) {
	if(argc == 3){
		shm_name = argv[1];
		shm_size = atoi(argv[2]);
		memory.consumers_total = 0;
		memory.producers_total = 0;
		memory.consumer_semaphore = sem_open("consumer_semaphore",O_CREAT | O_EXCL, S_IRWXU, shm_size - 1);
		memory.producer_semaphore = sem_open("producer_semaphore",O_CREAT | O_EXCL, S_IRWXU, shm_size - 1);
	}else{
		printf("%s\n", "You must write 2 arguments: buffer name and buffer size.");
		exit(1);
	}

	createShrareMemoryBlock(shm_name, sizeof(struct shm_block));
	writeShrareMemoryBlock(shm_name, &memory, 0);
	int n = * (int *) readShrareMemoryBlock(shm_name, sizeof(int), sizeof(int) * 3);
	deleteShrareMemoryBlock(shm_name);
	sem_unlink("consumer_semaphore");
	sem_unlink("producer_semaphore");
	return 0;
}