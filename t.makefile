.PHONY: run all clean

CFLAGS = -I./include -I ../../include -O0 -g -Wall -m64 -fms-extensions -ffreestanding -fno-stack-protector -std=gnu99

DIR = obj

OBJS = obj/test.o 

LIBS = ../../lib/libpacketngin.a

all: $(OBJS)
	ld -melf_x86_64 -nostdlib -e main -o main $^ $(LIBS)

obj/%.o: src/%.c
	mkdir -p $(DIR)
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -rf obj
	rm -f main

run: all
	../../bin/console script
