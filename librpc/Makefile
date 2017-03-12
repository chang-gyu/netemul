.PHONY: all clean

all: rpc_netemul.c
	gcc -Wall -I ../include -I ~/Project/rtos/include -DLINUX -c rpc_netemul.c  -fms-extensions
	ar rc librpc_netemul.a rpc_netemul.o

clean:
	rm librpc_netemul.a
