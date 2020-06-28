#include <shmhandler.h>

void createSemaphore(char *name, int value);

int main(int argc, char *argv[]) 
{
	if(argc != 3)
	{
		printf("\033[1;31m");
		printf("%s\n", "Error: you must write 2 arguments, buffer name and size");
		printf("\033[0m");
		exit(EXIT_FAILURE);
	}
	//-------------------------------------------------------------------------------------------------------
	// Initializes the buffer and the corresponding semaphores for producers and consumers
	//-------------------------------------------------------------------------------------------------------
	char *buffer_name = argv[1];
	int buffer_lenght = atoi(argv[2]);

	createShareMemoryBlock(buffer_name, buffer_lenght * sizeof(struct Message));

	char *producers_sem_name = generateTagName(buffer_name, PRODUCER_SEM_TAG);
	createSemaphore(producers_sem_name, buffer_lenght);

	char *consumers_sem_name = generateTagName(buffer_name, CONSUMER_SEM_TAG);
	createSemaphore(consumers_sem_name, 0);

	//-------------------------------------------------------------------------------------------------------
	// Initializes producers shared variables and his corresponding semaphore
	//-------------------------------------------------------------------------------------------------------
	struct shm_producers shmp;
	shmp.producers_total = 0;
	shmp.buffer_index = 0;
	shmp.buffer_isActive = 1;
	shmp.produced_messages = 0;
    shmp.accum_producers = 0;
	shmp.total_waited_time = 0;
	shmp.total_blocked_time = 0;
	shmp.total_kernel_time = 0;

	char *shmp_name = generateTagName(buffer_name, PRODUCER_SHM_TAG);
	createShareMemoryBlock(shmp_name, sizeof(struct shm_producers));
	struct shm_producers * shm_producers_ptr = (struct shm_producers *) mapShareMemoryBlock(shmp_name);
	writeInShareMemoryBlock(shm_producers_ptr, &shmp, sizeof(struct shm_producers), 0);

	char *shmp_sem_name = generateTagName(buffer_name, PRODUCER_SHM_SEM_TAG);
	createSemaphore(shmp_sem_name, 1);

	//-------------------------------------------------------------------------------------------------------
	// Initializes consumers shared variables and his corresponding semaphore
	//-------------------------------------------------------------------------------------------------------
	struct shm_consumers shmc;
	shmc.consumers_total = 0;
	shmc.buffer_index = 0;
	shmc.accum_consumers = 0;
	shmc.key_deleted = 0;
	shmc.total_waited_time = 0;
	shmc.total_blocked_time = 0;
	shmc.total_user_time = 0;

	char *shmc_name = generateTagName(buffer_name, CONSUMER_SHM_TAG);
	createShareMemoryBlock(shmc_name, sizeof(struct shm_consumers));
	struct shm_consumers * shm_consumers_ptr = mapShareMemoryBlock(shmc_name);
	writeInShareMemoryBlock(shm_consumers_ptr, &shmc, sizeof(struct shm_consumers), 0);

	char *shmc_sem_name = generateTagName(buffer_name, CONSUMER_SHM_SEM_TAG);
	createSemaphore(shmc_sem_name, 1);

	//-------------------------------------------------------------------------------------------------------

	free(producers_sem_name);
	free(consumers_sem_name);
	free(shmp_name);
	free(shmp_sem_name);
	free(shmc_name);
	free(shmc_sem_name);

	//deleteShareMemoryBlock(buffer_name);
	//sem_unlink(PRODUCER_SEM);
	//sem_unlink(CONSUMER_SEM);

	return 0;
}

void createSemaphore(char *name, int value)
{
	if (sem_open(name, O_CREAT | O_EXCL, S_IRWXU, value) == SEM_FAILED)
	{
		perror("sem_open(3) error");
        exit(EXIT_FAILURE);
	}
}