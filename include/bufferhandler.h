#ifndef _BUFFERHNADLER_H 
#define _BUFFERHNADLER_H  

void createShrareMemoryBlock(char * buffer_name, int size);
void * readShrareMemoryBlock(char * buffer_name, int bytes, int offset);
void writeShrareMemoryBlock(char * buffer_name, void * data, int offset);
void deleteShrareMemoryBlock(char * buffer_name);

#endif