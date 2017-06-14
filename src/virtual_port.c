#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "virtual_port.h"
#include "manager.h"
#include "composite.h"

extern Port* _port_create();

static void destroy(Node* this) {
	VirtualPort* port = (VirtualPort*)this;

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
	VirtualPort* port = (VirtualPort*)this;

	if(!port->is_active || !port->owner->is_active) {
		free_func(packet);
		return;
	}

	write(port->fd, packet->buffer, packet->end - packet->start);
	free_func(packet);

	return;
}

Port* virtual_port_create() {
	VirtualPort* port = (VirtualPort*)_port_create();
	if(!port)
		return NULL;

	port = realloc(port, sizeof(VirtualPort));
	if(!port)
		goto failed;

	port->type = NODE_TYPE_VIRTUAL_PORT;

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

