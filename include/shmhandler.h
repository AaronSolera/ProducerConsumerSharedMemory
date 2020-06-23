#ifndef _SHMHANDLER_H 
#define _SHMHANDLER_H  


void createShareMemoryBlock(char * buffer_name, int size);
void * mapShareMemoryBlock(char * buffer_name);
void writeInShareMemoryBlock(void * ptr, void * data, int size, int offset);
void deleteShareMemoryBlock(char * buffer_name);
int getShareMemoryBlockSize(char * buffer_name);

#endif