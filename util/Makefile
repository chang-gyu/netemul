.PHONY: all clean 

CFLAGS = -I ../../rtos/include -I../include  -I ../librpc -O2 -g -Wall -Werror -m64 -std=gnu99 -fms-extensions -D__LINUX
LIBS =  ../librpc/librpc_netemul.a ../libumpn.a

all: help ifconfig list tree create destroy on off set get manager

help: src/help.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

ifconfig: src/ifconfig.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

tree: src/tree.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

list: src/list.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

create: src/create.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

destroy: src/destroy.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

on: src/on.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

off: src/off.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

set: src/set.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

get: src/get.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

manager: src/rpc_manager.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f help
	rm -f ifconfig
	rm -f tree
	rm -f list
	rm -f create
	rm -f destroy
	rm -f on
	rm -f off
	rm -f set
	rm -f get
	rm -f manager
