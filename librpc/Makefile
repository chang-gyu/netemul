.PHONY: all clean

all: rpc_netemul.c
	gcc -std=gnu99 -g -Wall -I ../include -I ../sdk -DLINUX -D__LINUX -c rpc_netemul.c  -fms-extensions
	ar rc librpc_netemul.a rpc_netemul.o

clean:
	rm rpc_netemul.o
	rm librpc_netemul.a
