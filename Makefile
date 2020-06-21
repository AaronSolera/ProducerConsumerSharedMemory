CC = gcc
MAKE_STATIC_LIB = ar rv
LIB = cd ./lib &&
RM_O = cd ./lib && rm *.o

.PHONY: call

call: producer initializer
	$(RM_O)

producer: libshmhandler.a
	$(CC) -o ./bin/producer ./src/producer.c -I./include ./lib/libshmhandler.a -lm -lrt -pthread

initializer: libshmhandler.a
	$(CC) -o ./bin/initializer ./src/initializer.c -I./include ./lib/libshmhandler.a -lm -lrt -pthread

libshmhandler.a: shmhandler.o
	$(LIB) $(MAKE_STATIC_LIB) libshmhandler.a shmhandler.o

shmhandler.o:
	$(LIB) $(CC) -c shmhandler.c -I../include