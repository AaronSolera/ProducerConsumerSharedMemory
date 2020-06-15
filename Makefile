CC = gcc
MAKE_STATIC_LIB = ar rv
LIB = cd ./lib &&
RM_O = && cd ./lib && rm *.o

.PHONY: producer

producer: libbufferhandler.a
	$(CC) -o ./bin/producer ./src/producer.c -I./include ./lib/libbufferhandler.a -lrt $(RM_O)

libbufferhandler.a: bufferhandler.o
	$(LIB) $(MAKE_STATIC_LIB) libbufferhandler.a bufferhandler.o

bufferhandler.o:
	$(LIB) $(CC) -c bufferhandler.c -I../include