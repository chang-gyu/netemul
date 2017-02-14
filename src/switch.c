#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "manager.h"
#include "port.h"
#include "ether_switch.h"
#include "hub_switch.h"

bool switch_unicast(Port* port, Packet* packet) {
	if(!port->out) 
		return false;

#ifdef NET_CONTROL
	if(!fifo_push(port->out->queue, packet))
		return false;
#else
	port->out->send(port->out, packet);
#endif

	return true;
}

bool switch_multicast(Port* port, Packet* packet) {
	// Not yet
	return true;
}

bool switch_broadcast(Port* port, Packet* packet) {
	Packet* packet_dup(Packet* packet) {
		uint16_t len = packet->end - packet->start;

		Packet* p = (Packet*)malloc(sizeof(Packet) + len);
		if(!p)
			return NULL;

		uint16_t start = packet->start;
		uint16_t size = packet->size;
		memcpy(p, packet, sizeof(Packet));
		p->start = start;
		p->size = size;
		p->end = p->start + len;

		memcpy(p->buffer + p->start, packet->buffer + packet->start, len);

		return p;
	}

	Switch* s = (Switch*)port->owner;

	for(int i = 0; i < s->node_count; i++) {
		if(!s->nodes[i])
			continue;

		/* Destination */
		Port* dst = (Port*)s->nodes[i];
		if(dst == port) 
			// Do not send to myself
			continue;

		/* Packet duplication */
		Packet* dup_packet = packet_dup(packet);
		if(!dup_packet) 
			return false;

		if(!switch_unicast(dst, dup_packet)) {
			free(dup_packet);
			continue;
		}
	}

	/* Free original packet */
	free(packet);
	return true;
}

Switch* switch_create(int port_count, int type) {
	printf("debug1\n");
	Switch* s;
	char* name;

	switch(type) {
		/* Switch is abstract class. There is no normal switch. */
		case NODE_TYPE_ETHER_SWITCH:
			if(!(s = ether_switch_create(port_count)))
				return NULL;

printf("debug\n");
			name = s->name;
			name[0] = 's'; // 'Switch'
			break;
/*
		case NODE_TYPE_HUB_SWITCH:
			if(!(s = hub_create(port_count)))
				return NULL;

			name = s->name;
			name[0] = 'h'; // 'Hub'
			break;
*/
		default:
			return NULL;
	}

	/* Register switch to network emulator manager */
	bool result = false;
	for(int i = 0; i < MAX_NODE_COUNT; i++) {
		sprintf(&name[1], "%d", i);
		
		if(!get_node(name)) {
			result = node_register((Composite*)s, name);
			break;
		}
	}

	if(!result) {
		s->destroy((Node*)s);
		return NULL;
	}

	return s;
}

