/*
*	Producer message format
*
*	id: 		  0000000000 (10 bytes)
*	date:         01/01/2020 (10 bytes)
*	time:         00:00:00 (8 bytes)
*	magic_number: 1 (1 byte)
*
*	Example: 00000000000-01/01/2020-00:00:00-1
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>

#define PRODUCER_SEM_TAG		"_producers_sem"
#define CONSUMER_SEM_TAG		"_consumers_sem"
#define PRODUCER_SHM_TAG  		"_producers_shm"
#define CONSUMER_SHM_TAG  		"_consumers_shm"
#define PRODUCER_SHM_SEM_TAG	"_consumers_shm_sem"
#define CONSUMER_SHM_SEM_TAG	"_producers_shm_sem"
#define MAX_MAGIC_NUMBER		6

struct Date 
{
	int day;
	int month;
	int year;
};

struct Time 
{
	int hour;
	int minutes;
	int seconds;
};

struct Message 
{
	pid_t id;
	struct Date date;
	struct Time time;
	int magic_number;
};

struct shm_producers 
{
	int producers_total;
	int buffer_index;
	int buffer_isActive;
};

struct shm_consumers 
{
	int consumers_total;
	int buffer_index;
};

char * generateTagName(char *name, const char *tag);

sem_t * openSemaphore(char *name);

void * readFromShareMemoryBlock(char *buffer_name);

//---------------------------------------------------------------------------------------------------

char * generateTagName(char *name, const char *tag)
{
	char *tag_name = (char *) calloc(strlen(name) + strlen(tag), sizeof(char));
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
	void * ptr = mmap(NULL, shm_obj.st_size, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED)
	{
	  printf("Map failed in read process: %s\n", strerror(errno));
	  exit(1);
	}

	return ptr;
}
