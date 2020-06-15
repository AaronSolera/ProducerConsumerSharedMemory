#include <stdio.h>
#include <stdlib.h>
#include <bufferhandler.h>

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
	/*
	createBuffer(shm_name, 8);
	writeBuffer(shm_name, "HolaCoca", 0);
	char * msg = readBuffer(shm_name, 8, 0);
	printf("%s\n", msg);
	writeBuffer(shm_name, "Mota", 4);
	msg = readBuffer(shm_name, 8, 0);
	printf("%s\n", msg);
	free(msg);
	deleteBuffer(shm_name); 
	*/
	return 0;
}