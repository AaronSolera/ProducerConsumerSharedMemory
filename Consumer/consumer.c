#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>        // For mode constants
#include <fcntl.h>           // For O_* constants
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

struct Consumer
{
	int PID;
	int op_mode;
} 
consumer;

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

	return 0;
}
