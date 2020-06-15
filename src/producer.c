#include <stdio.h>
#include <stdlib.h>
#include <bufferhandler.h>

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
	createBuffer(shm_name, 1024);
	writeBuffer(shm_name, "Pamelópolis\n");
	printf("%s", readBuffer(shm_name));
	return 0;
}