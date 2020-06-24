/*  
*   Programar en C: Memoria Compartida POSIX. (Shared Memory). Linux
*   This code is a modification obtained at the website https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
*   Creator: WhileTrueThenDream, Mar 4th, 2020.
*/
#include "shmhandler.h"
#define ERROR -1

int fd;
char *ptr;
struct stat shm_obj;

void createShareMemoryBlock(char * buffer_name, int size) 
{
	// Open and create shared memory buffer with shm_open syscall. It returns a file descriptor.
	fd = shm_open (buffer_name, O_CREAT | O_RDWR  ,00700); 
	if(fd == ERROR) {
	   printf("Error creating shared memory buffer.\n");
	   exit(1);
	}
	// Sizing the memory to be used with ftruncate syscall.
	if(ftruncate(fd, size) == ERROR) {
	   printf("Error shared memory buffer cannot be sized.\n");
	   exit(1);
	}
}

void * mapShareMemoryBlock(char * buffer_name){
	// Open shared memory buffer to be written with shm_open syscall. It returns a file descriptor.
	fd = shm_open (buffer_name,  O_RDWR  , 00200); 
	if(fd == ERROR)
	{
	   printf("Error file descriptor %s\n", strerror(errno));
	   exit(1);
	}
	// Getting the shared memory object struct for getting the shared memory buffer size.
	if(fstat(fd, &shm_obj) == ERROR) {
	   printf("Error getting stat struct.\n");
	   exit(1);
	}
	// Mapping the shared memory buffer for writing into it.
	void * ptr = mmap(NULL, shm_obj.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
	{
	  printf("Map failed in write process: %s\n", strerror(errno));
	  exit(1);
	}
	return ptr;
}

int getShareMemoryBlockSize(char * buffer_name){
	// Open shared memory buffer to be written with shm_open syscall. It returns a file descriptor.
	fd = shm_open (buffer_name,  O_RDWR  , 00200); 
	if(fd == ERROR)
	{
	   printf("Error file descriptor %s\n", strerror(errno));
	   exit(1);
	}
	// Getting the shared memory object struct for getting the shared memory buffer size.
	if(fstat(fd, &shm_obj) == ERROR) {
	   printf("Error getting stat struct.\n");
	   exit(1);
	}
	return shm_obj.st_size;
}

void writeInShareMemoryBlock(void * ptr, void * data, int size, int offset) 
{
	// Copy the data given by the message parameter into the shared memory buffer position offset.
	memcpy(ptr + (offset * size), data, size);
}

void deleteShareMemoryBlock(char * buffer_name) 
{
	// Setting free the shared memory buffer with shm_unlink syscall.
	shm_unlink(buffer_name);
}

int not(int boolean)
{
	return 1 - boolean;
}

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