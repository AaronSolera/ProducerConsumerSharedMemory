#ifndef _BUFFERHNADLER_H 
#define _BUFFERHNADLER_H  

void createShareMemoryBlock(char * buffer_name, int size);
void * readFromShareMemoryBlock1(char * buffer_name, int bytes, int offset);
void writeInShareMemoryBlock(char * buffer_name, void * data, int offset);
void deleteShareMemoryBlock(char * buffer_name);

#endif