#include <stdio.h>
#include <stdlib.h>	//exit 
#include <string.h>
#include <readline.h>
#include <unistd.h>
#include <malloc.h>

#include <util/cmd.h>
#include <util/event.h>
#include <net/nic.h>
#include <net/udp.h>
#include <net/packet.h>

#include "manager.h"
#include "command.h"
#include "network.h"

/* Packet processing buffer */
#define PACKET_BUF_SIZE		2048
#define ALIGN
static bool input_process(void* context) {
	Manager* manager = get_manager();

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	fd_set temp = manager->read_fds;
	int ret = select(manager->nfds, &temp, NULL, NULL, &tv);

	if(ret == -1) {
		perror("Selector error");
		exit(-1);
	} else if(ret) {
		for(int i = 0; i < manager->fd_count; i++) {
			int fd = (int64_t)list_get(manager->fds, i);

			if(FD_ISSET(fd, &temp) != 0) {
				if(fd == STDIN_FILENO) {
					/* Process input command */
					command_process(fd);
				} else {
					/* Process input packet */
					Packet* packet = (Packet*)malloc(sizeof(Packet) + PACKET_BUF_SIZE + (ALIGN -1));
					if(!packet)
						continue;

					packet->start = (((uint64_t)packet->buffer + (ALIGN - 1)) & ~(ALIGN - 1)) - (uint64_t)packet->buffer;

					Port* port = manager->nis[fd]->port;
					int size = read(fd, packet->buffer + packet->start, PACKET_BUF_SIZE);
					if(size < 0) {
						port->destroy((Node*)port);
						perror("Error\n");
						free(packet);
						continue;
					}

					packet->end = packet->start + size;

					// NOTE: fd is same as network interface index.
					network_process(port, packet);
				}
			}
		}
	}

	return true;
}

bool input_init() {
	/* Commands & packet input processing */
	if(!fd_add(STDIN_FILENO))
		return false;

	event_busy_add(input_process, NULL);

	return true;
}
