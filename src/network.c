#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <net/packet.h>
//#include <net/ether.h>
//#include <net/ip.h>
#include <util/event.h>

#include "port.h"
#include "manager.h"
#include "composite.h"

//static int debug = 0;
void network_process(VirtualPort* port, Packet* packet) {
	if(!port->is_active || !port->owner->is_active)
		goto failed;

	if(!port->out)
		goto failed;
/*
	Ether* ether = (Ether*)(packet->buffer + packet->start);

	if(endian16(ether->type) == ETHER_TYPE_IPv4) {
		IP* ip = (IP*)ether->payload;

		if(ip->protocol == IP_PROTOCOL_ICMP) {
			if(!strcmp("p0.0", port->name)) {
				printf("port: %s debug: %d\n", port->name, debug++);
				packet_dump(packet);
			}
		}
	}
*/
	if(!fifo_push(port->out->queue, packet))
		goto failed;

	return;

failed:
	free_func(packet);
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
			component->packet_forward(component, packet);

			return true;
		}
	}

	return true;
}

void network_init() {
	/* Network control start */
	event_busy_add(network_control, NULL);
}

