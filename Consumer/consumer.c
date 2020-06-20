#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>        // For mode constants
#include <fcntl.h>           // For O_* constants
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>


int main(int argc, char *argv[])
{
	consumer.PID = getpid();
	consumer.op_mode = atoi(argv[3]);

	char *buffer_name = argv[1];

	printf("\033[1;31m");
	printf("Process ID     : %i\n", consumer.PID);
	printf("\033[1;33m");
	printf("Operation mode : %i\n", consumer.op_mode);
	printf("\033[0m");

	int fd;
	char *ptr_shm;
	struct stat shmobj_st;

	fd = shm_open(buffer_name, O_RDONLY, 00400);

	if (fd == -1)
	{
		printf("Error in file descriptor: %s\n", strerror(errno));
		exit(1);
	}

	if (fstat(fd, &shmobj_st) == -1)
	{
		printf("Error fstat\n");
		exit(1);
	}

	ptr_shm = mmap(NULL, shmobj_st.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr_shm == MAP_FAILED)
	{
		printf("Map failed in read mapping process: %s\n", strerror(errno));
		exit(1);	
	}

	printf("%s\n", ptr_shm);

	close(fd);

/*
	char *buffer_name = argv[1];

	int buffer_size = atoi(argv[2]);

	char *shmp_name = malloc(strlen(buffer_name) + sizeof(producer_tag));
	strcpy(shmp_name, buffer_name);
	strcat(shmp_name, producer_tag);

	char *shmp_sem_name = malloc(strlen(shmp_name) + sizeof(semaphore_tag));
	strcpy(shmp_sem_name, shmp_name);
	strcat(shmp_sem_name, semaphore_tag);

	char *shmc_name = malloc(strlen(buffer_name) + sizeof(consumer_tag));
	strcpy(shmc_name, buffer_name);
	strcat(shmc_name, consumer_tag);

	char *shmc_sem_name = malloc(strlen(shmc_name) + sizeof(semaphore_tag));
	strcpy(shmc_sem_name, shmc_name);
	strcat(shmc_sem_name, semaphore_tag);

	printf("%s\n", buffer_name);
	printf("%s\n", shmp_name);
	printf("%s\n", shmp_sem_name);
	printf("%s\n", shmc_name);
	printf("%s\n", shmc_sem_name);

	free(shmp_name);
	free(shmp_sem_name);
	free(shmc_name);
	free(shmc_sem_name);
*/

	return 0;
}
