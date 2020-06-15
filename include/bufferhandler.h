#ifndef _BUFFERHNADLER_H 
#define _BUFFERHNADLER_H  

void createBuffer(char * buffer_name, int size);
char * readBuffer(char * buffer_name);
void writeBuffer(char * buffer_name, char * message);

#endif