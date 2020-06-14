#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	char *buffer_name = argv[1];

	int buffer_size = atoi(argv[2]);

	//---------------------------------------------------------------------
	// Initializer
	//---------------------------------------------------------------------

	int fd;

	fd = shm_open(buffer_name, O_CREAT | O_RDWR, 00600);

	if (fd == -1)
	{
		printf("Error, shared memory could not be created\n");
		exit(1);
	}

	if(ftruncate(fd, buffer_size) == -1)
	{
		printf("Error, shared memory could not be resized\n");
		exit(1);
	}

	printf("Block size     : %d\n", buffer_size);

	//---------------------------------------------------------------------
	// Producer
	//---------------------------------------------------------------------

	char message[] = "Hello World!\n";

	char *ptr_shm;

	ptr_shm = mmap(NULL, sizeof(message), PROT_WRITE, MAP_SHARED, fd, 0);

	if (ptr_shm == MAP_FAILED)
	{
		printf("Error in memory mapping\n");
		exit(1);
	}

	memcpy(ptr_shm, message, sizeof(message));

	printf("Message to write : %s\n", message);
	printf("Message size     : %d\n", (int)sizeof(message));
	printf("Writing process complete\n");

	close(fd);

	return 0;
}