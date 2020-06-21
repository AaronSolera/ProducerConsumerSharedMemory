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
#define TRUE 					1
#define FALSE 					0

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
