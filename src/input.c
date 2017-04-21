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
#include "port.h"
#include "composite.h"

/* Packet processing buffer */
#define PACKET_BUF_SIZE		2048

static void network_process(Port* port, Packet* packet) {
	if(!port->is_active || !port->owner->is_active)
		goto failed;

	if(!port->out)
		goto failed;

	if(!fifo_push(port->out->queue, packet))
		goto failed;

	return;

failed:
	free_func(packet);
}

static bool input_process(void* context) {
    Manager* manager = get_manager();

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
                    printf("??");
                    int size = read(fd, packet->buffer, PACKET_BUF_SIZE);
                    printf("1?");
                    free_func(packet);
                    printf("2?");

                    //packet->start = 0;
                    //packet->end = packet->start + size;

                    //TODO: Distribute VirutalPort & PhysicalPort.
                    // NOTE: fd is same as network interface index.
//                    VirtualPort* port = manager->nis[fd]->port; // 여기서 메모리 덤프 발생. Port* port라고 하면 안됨.. 참조를 못함.
                    //network_process(port, packet);
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
