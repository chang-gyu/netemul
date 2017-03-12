.PHONY: all clean

all: rpc_netemul.c
	gcc -Wall -I ../include -I ~/Project/rtos/include -c rpc_netemul.c
	ar rc librpc_netemul.a rpc_netemul.o

clean:
