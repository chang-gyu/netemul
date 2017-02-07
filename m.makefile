.PHONY: run all clean

CFLAGS = -I./include -I ../../include -O2 -g -Wall -m64 -fms-extensions -ffreestanding -fno-stack-protector -std=gnu99

DIR = obj

OBJS = obj/main.o obj/manager.o obj/input.o obj/command.o obj/network.o obj/ni.o obj/endpoint.o obj/endpoint_port.o obj/host.o obj/composite.o obj/component.o obj/port.o obj/cable.o obj/link.o obj/switch.o obj/ether_switch.o

LIBS = ../../lib/libpacketngin.a

all: $(OBJS)
	ld -melf_x86_64 -nostdlib -e main -o netemul $^ $(LIBS)

obj/%.o: src/%.c
	mkdir -p $(DIR)
	gcc $(CFLAGS) -c -o $@ $< -DNETEMUL_PACKETNGIN -DNET_CONTROL
	#NETEMUL_PACKETNGIN < For PacketNgin
	#NET_CONTROL < QoS configration

clean:
	rm -rf obj
	rm -f main

run: all
	../../bin/console script
