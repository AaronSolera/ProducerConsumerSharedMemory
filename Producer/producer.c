#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char * shm_name;
int seconds_mean;

int main(int argc, char *argv[]) {
	if(argc == 3){
		shm_name = argv[1];
		seconds_mean = atoi(argv[2]);
	}else{
		printf("%s\n", "You must write 2 arguments: buffer name and seconds mean.");
		exit(1);
	}
	return 0;
}