#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>   
#include <fcntl.h>           
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*  
*   Programar en C: Memoria Compartida POSIX. (Shared Memory). Linux
*   This code is a modification obtained at the website https://github.com/WhileTrueThenDream/ExamplesCLinuxUserSpace
*   Creator: WhileTrueThenDream, Mar 4th, 2020.
*/

#define ERROR -1

int fd;
char *ptr;
struct stat shm_obj;

void createShareMemoryBlock(char * buffer_name, int size) 
{
	// Open and create shared memory buffer with shm_open syscall. It returns a file descriptor.
	int fd = shm_open (buffer_name, O_CREAT | O_RDWR  ,00700); 
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
	int fd = shm_open (buffer_name,  O_RDWR  , 00200); 
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