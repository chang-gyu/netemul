#include <stdio.h>
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

static bool input_process(void* context) {
	Manager* manager = get_manager();

#ifdef __LINUX
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	fd_set temp = manager->read_fds;
	int ret = select(manager->nfds, &temp, NULL, NULL, &tv); 

	if(ret == -1) {
		perror("Selector error");
		return false;
	} else if(ret) {
		for(int i = 0; i < manager->fd_count; i++) {
			int fd = (int64_t)list_get(manager->fds, i);

			if(FD_ISSET(fd, &temp) != 0) {
				if(fd == STDIN_FILENO) {
					/* Process input command */
					command_process(fd);
				} else {
					/* Process input packet */
					Packet* packet = (Packet*)malloc(sizeof(Packet) + PACKET_BUF_SIZE);
					int size = read(fd, packet->buffer, PACKET_BUF_SIZE); 

					packet->start = 0;
					packet->end = packet->start + size;
					
					// NOTE: fd is same as network interface index.	
					EndPointPort* port = manager->nis[fd]->port;
					network_process(port, packet);
				}
			}
		}
	} 
#else
	uint32_t i = 0;
	uint32_t count = nic_count();
	for(i = 0; i < count; i++) {
		if(manager->nis[i]->used == 0)
			continue;

		NIC* nic = nic_get(i);
		if(nic_has_input(nic)) {
			Packet* packet = nic_input(nic);

			// NOTE: fd is same as network interface index. 
			EndPointPort* port = manager->nis[i]->port;
			network_process(port, packet);
		}
	}
#endif

	return true;
}

#ifndef __LINUX
static bool cli_process(void* context) {
	char *name = readline();
	if(!name)
		return true;
	else {
		int len = strlen(name);
		if(len > 0) {
//			printf("command input : %s\n", name);
			cmd_exec(name, NULL);
			fflush(stdout);
		}
	}
	return true;
}
#endif

bool input_init() {
	/* Commands & packet input processing */
#ifdef __LINUX
	if(!fd_add(STDIN_FILENO))
		return false;
#else
	event_busy_add(cli_process, NULL);
#endif

	event_busy_add(input_process, NULL);

	return true;
}
