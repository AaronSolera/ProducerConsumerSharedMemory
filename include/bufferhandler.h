#ifndef _BUFFERHNADLER_H 
#define _BUFFERHNADLER_H  

void createBuffer(char * buffer_name, int size);
char * readBuffer(char * buffer_name, int bytes, int offset);
void writeBuffer(char * buffer_name, char * message, int offset);
void deleteBuffer(char * buffer_name);

#endif