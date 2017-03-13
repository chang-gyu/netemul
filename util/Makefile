.PHONY: all clean 

CFLAGS = -I ../../include -I../include  -I ../librpc -O2 -g -Wall -Werror -m64 -std=gnu99 -fms-extensions -DLINUX
LIBS =  ../librpc/librpc_netemul.a ../libumpn.a

all: list create on manager

list: src/list.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

create: src/create.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

on: src/on.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

manager: src/rpc_manager.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f list
	rm -f create
	rm -f on
	rm -f manager
