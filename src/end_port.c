#include <unistd.h>
#include <string.h>
#include <malloc.h>

#include "end_port.h"
#include "manager.h"
#include "composite.h"

extern Port* _port_create();

static void destroy(Node* this) {
	EndPort* port = (EndPort*)this;

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
	EndPort* port = (EndPort*)this;

	if(!port->is_active || !port->owner->is_active) {
		free(packet);
		return;
	}

	write(port->fd, packet->buffer, packet->end - packet->start);
	free(packet);
}

Port* end_port_create() {
	EndPort* port = (EndPort*)_port_create();
	if(!port)
		return NULL;

	port = realloc(port, sizeof(EndPort));
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

