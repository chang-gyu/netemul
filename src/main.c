#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include <util/event.h>

#include "version.h"
#include "manager.h"
#include "command.h"

static void help() {
	printf("Usage: netemul [Options]\n");
	printf("Option:\n");
	printf("\t-h --help		: Show help\n");
	printf("\t-v --version		: Show version\n");
	printf("\t-s --script		: Script file path\n");
}

int main(int argc, char** argv) {
	if(geteuid() != 0) {
		printf("Permssion denied : $./sudo netemul \n");
		return -1;
	}

	/* Process option for program */
	static const struct option options[] = {
		{ .name = "help", .val = 'h' },
		{ .name = "version", .val = 'v' },
		{ .name = "script", .has_arg = required_argument, .val = 's' },
		{ 0 }
	};

	char* script;
	int opt;

	while((opt = getopt_long(argc, argv, "hvs:", options, NULL)) != -1) {
		switch(opt) {
			case 'h':
				help();
				return 0;

			case 'v':
				printf("PacketNgin Network Emulator ver %d.%d.%d-%s\n",
						VERSION_MAJOR, VERSION_MINOR,
						VERSION_MICRO, VERSION_TAG);
				return 0;

			case 's':
				script = optarg;
				break;

			default:
				printf("Unknown option '%c'\n", opt);
				help();
				return -1;
		}

	}

	/* Create network emulator manager */
	if(!manager_init())
		return -1;

	printf("\nWelcome to PacketNgin Network Emulator\n\n");

	/* Execute commands in script */
	if(script) {
		int fd = open(script, O_RDONLY);
		if(fd < 0) {
			perror("Cannot open script file");
		} else {
			command_process(fd);
			close(fd);
		}
	}

	/* Event machine start */
	while(1)
		event_loop();

	return 0;
}
