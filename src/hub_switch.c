#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "hub_switch.h"
#include "switch.h"

static void hub_send(Component* this, Packet* packet) {
	if(!this->is_active || !this->owner->is_active) {
		printf("Node %s is inactive\n", this->name);
		goto failed;
	}

	if(!switch_broadcast((Port*)this, packet))
		goto failed;

	return;

failed:
	free(packet);
}

Switch* hub_create(int port_count) {
	HubSwitch* hub = malloc(sizeof(HubSwitch));
	if(!hub)
		return NULL;

	memset(hub, 0x0, sizeof(HubSwitch));

	hub->type = NODE_TYPE_HUB_SWITCH;
	hub->node_count = port_count;

	/* Inherit */
	if(!composite_inherit((Composite*)hub))
		goto failed;

	/* Extends */
	// Nothing

	/* Method overriding */
	for(int i = 0; i < hub->node_count; i++)
		hub->nodes[i]->packet_forward = hub_send;

	return (Switch*)hub;

failed:
	hub->destroy((Node*)hub);
	return NULL;
}
