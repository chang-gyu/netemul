#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include <net/packet.h>
#include "endpoint_port.h"
#include "manager.h"
#include "composite.h"

//#include "dump.h" //debug

extern Port* _port_create();

static void destroy(Node* this) {
	EndPointPort* port = (EndPointPort*)this;

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

static void send(Component* this, Packet* packet) {
	EndPointPort* port = (EndPointPort*)this;

	if(!port->is_active || !port->owner->is_active) {
#ifdef NETEMUL_PACKETNGIN
		nic_free(packet);
#else
		free(packet);
#endif
		return;
	}

#ifdef NETEMUL_PACKETNGIN
	NIC* nic = port->ni->nic;
	printf("nic : %p\n", nic);
	packet_dump(packet);
	if(!nic_output(nic, packet)) {
		printf("nic error\n");
		nic_free(packet);
	}
#else
	write(port->fd, packet->buffer, packet->end - packet->start);
	free(packet);
#endif

	return;
}

Port* endpoint_port_create() {
	EndPointPort* port = (EndPointPort*)_port_create();
	if(!port)
		return NULL;

	port = realloc(port, sizeof(EndPointPort));
	if(!port)
		goto failed;

	port->type = NODE_TYPE_END_PORT;

	/* Extends */ 
	// Nothing

	/* Method overriding */
	port->send = send;
	port->destroy = destroy;

	return (Port*)port;

failed:
	port->destroy((Node*)port);
	return NULL;

}

