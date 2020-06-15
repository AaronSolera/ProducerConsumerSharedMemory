#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>   
#include <fcntl.h>           
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define ERROR -1

int fd;
char *ptr;

void createBuffer(char * buffer_name, int size) {
	fd = shm_open (buffer_name, O_CREAT | O_RDWR  ,00700); /* create s.m object*/
	if(fd == ERROR) {
	   printf("Error creating shared memory buffer.\n");
	   exit(1);
	}
	if(ftruncate(fd, size) == ERROR) {
	   printf("Error shared memory buffer cannot be sized.\n");
	   exit(1);
	}
	close(fd);
}

char * readBuffer(char * buffer_name, int bytes, int offset) {
	struct stat shm_obj;
	fd = shm_open (buffer_name,  O_RDONLY  , 00400); /* open s.m object*/
	char * read = (char *) calloc(bytes, sizeof(char));
	if(fd == ERROR) {
	   printf("Error openning shared memory buffer: %s\n", strerror(errno));
	   exit(1);
	}
	if(fstat(fd, &shm_obj) == ERROR) {
	   printf("Error getting stat struct.\n");
	   exit(1);
	}
	ptr = mmap(NULL, shm_obj.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
	{
	  printf("Map failed in read process: %s\n", strerror(errno));
	  exit(1);
	}
	memcpy(read, ptr + offset, bytes);
	close(fd);
	return read;
}

void writeBuffer(char * buffer_name, char * message, int offset) {
	fd = shm_open (buffer_name,  O_RDWR  , 00200); /* open s.m object*/
	if(fd == ERROR)
	{
	   printf("Error file descriptor %s\n", strerror(errno));
	   exit(1);
	}
	ptr = mmap(NULL, strlen(message), PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED)
	{
	  printf("Map failed in write process: %s\n", strerror(errno));
	  exit(1);
	}
	memcpy(ptr + offset, message, strlen(message));
	close(fd);
}

void deleteBuffer(char * buffer_name) {
	shm_unlink(buffer_name);
}