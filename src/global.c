#include "global.h"

char * generateTagName(char *name, const char *tag)
{
	char *tag_name = (char *)malloc(strlen(name) + sizeof(tag));
	strcpy(tag_name, name);
	strcat(tag_name, tag);

	return tag_name;
}

sem_t * openSemaphore(char *name)
{
	sem_t * semaphore = sem_open(name, O_RDWR);

	if (semaphore == SEM_FAILED)
	{
		perror("sem_open(3) error");
        exit(EXIT_FAILURE);
	}

	return semaphore;
}

void * readFromShareMemoryBlock(char *buffer_name) 
{
	struct stat shm_obj;

	// Open shared memory buffer to be read with shm_open syscall. It returns a file descriptor.
	int fd = shm_open (buffer_name,  O_RDONLY  , 00400); 

	if(fd == -1) {
	   printf("Error openning shared memory buffer: %s\n", strerror(errno));
	   exit(1);
	}

	// Getting the shared memory object struct for getting the shared memory buffer size.
	if(fstat(fd, &shm_obj) == -1) {
	   printf("Error getting stat struct.\n");
	   exit(1);
	}

	// Mapping the shared memory buffer for accessing it.
	void *ptr = mmap(NULL, shm_obj.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED)
	{
	  printf("Map failed in read process: %s\n", strerror(errno));
	  exit(1);
	}

	return ptr;
}