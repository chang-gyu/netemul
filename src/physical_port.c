#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "virtual_port.h"
#include "manager.h"
#include "composite.h"
#include <net/ether.h>

extern Port* _port_create();

static void destroy(Node* this) {
	PhysicalPort* port = (PhysicalPort*)this;

	/* Overriding part */
	port_detach(port->ni);

	Component* src = port->in;
	Component* dst = port->out;
	if(src)
		src->out = NULL;
	if(dst)
		dst->in = NULL;

	if(port->owner)
		port->owner->node_count--;

	if(port->queue)
		fifo_destroy(port->queue);

	free(port);
	port = NULL;
}

static void packet_forward(Component* this, Packet* packet) {
	PhysicalPort* port = (PhysicalPort*)this;

	if(!port->is_active || !port->owner->is_active) {
		free_func(packet);
		return;
	}
    struct sockaddr_ll* saddrll = port->saddrll;
    Ether* ether = (Ether*)(packet->buffer + packet->start);

    memcpy((void*)(saddrll->sll_addr), (void*)(ether->dmac), ETH_ALEN);
	sendto(port->wfd, packet->buffer, packet->end - packet->start, 0, (struct sockaddr*)saddrll, sizeof(*saddrll));
	free_func(packet);

	return;
}

bool physical_send_port_create(PhysicalPort* port) {
    char* iface = port->ifname;
    int wfd;
    if((wfd = socket(AF_PACKET, SOCK_RAW, ETH_P_IP)) < 0) {
        printf("Error\n");
        return NULL;
    }

    struct ifreq buffer;
    int ifindex;
    memset(&buffer, 0x00, sizeof(buffer));
    strncpy(buffer.ifr_name, iface, IFNAMSIZ);
    if(ioctl(wfd, SIOCGIFINDEX, &buffer) < 0) {
        printf("error\n");
        close(wfd);
        return NULL;
    }
    ifindex = buffer.ifr_ifindex;

    struct sockaddr_ll* saddrll;
    saddrll = malloc(sizeof(struct sockaddr_ll));
    memset((void*)saddrll, 0, sizeof(struct sockaddr_ll));
    saddrll->sll_family = PF_PACKET;
    saddrll->sll_ifindex = port->fd;
    saddrll->sll_halen = ETH_ALEN;

    port->wfd = wfd;
    port->saddrll = saddrll;
    return true;
}
Port* physical_port_create() {
	PhysicalPort* port = (PhysicalPort*)_port_create();
	if(!port)
		return NULL;

	port = realloc(port, sizeof(PhysicalPort));
	if(!port)
		goto failed;

	port->type = NODE_TYPE_PHYSICAL_PORT;

	/* Extends */
	// Nothing

	/* Method overriding */
	port->packet_forward = packet_forward;
	port->destroy = destroy;

	return (Port*)port;

failed:
	port->destroy((Node*)port);
	return NULL;

}

