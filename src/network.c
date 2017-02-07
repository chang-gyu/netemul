#include <stdio.h>
#include <malloc.h>

#include <net/packet.h>
#include <util/event.h>

#include "port.h"
#include "manager.h"
#include "composite.h"

void network_process(EndPointPort* port, Packet* packet) {
	if(!port->is_active || !port->owner->is_active) 
		goto failed;

	if(!port->out) 
		goto failed;

#ifdef NET_CONTROL
	if(!fifo_push(port->out->queue, packet)) 
		goto failed;
#else
	port->out->send(port->out, packet);
#endif

	return;

failed:
#ifdef NETEMUL_PACKETNGIN
	nic_free(packet);
#else
	free(packet);
#endif
}


static bool network_control(void* context) {
	Manager* manager = get_manager();

	ListIterator iter;
	list_iterator_init(&iter, manager->components);

	while(list_iterator_has_next(&iter)) {
		Component* component = list_iterator_next(&iter);
		FIFO* queue = component->queue;
		Packet* packet;
		while((packet = (Packet*)fifo_pop(queue))) {
			component->send(component, packet);

			return true;
		}
		
	}

	return true;
}

void network_init() {
#ifdef NET_CONTROL
	/* Network control start */ 
	event_busy_add(network_control, NULL);
#endif
}

