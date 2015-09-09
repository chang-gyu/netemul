#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <net/ether.h>
#include <timer.h>

#include "ether_switch.h"
#include "switch.h"
#include "manager.h"

/**
 * MAC filtering table entity.
 */
typedef struct {
	Port*		port;		///< Destination port.
	uint64_t	mac;		///< Destination MAC address. 
	int		age;		///< Left time for living (seconds).
	time_t		timeout;	///< Deadline of entity (milli-seconds).
} MACTableEntity;

/* Show MAC filtering table */
static void list_table(EtherSwitch* s) {
	int get_portnum(Port* port) {
		for(int i = 0; i < s->node_count; i++) {
			if(!s->nodes[i])
				break;

			if(s->nodes[i] == (void*)port)
				return i;
		}

		return -1;
	}

	CacheIterator iter;
	cache_iterator_init(&iter, s->table);
	printf("\t\tMAC Filtering Table\n");
	printf("\t\t============================================\n");
	printf("\t\tPort\tMAC\t\t\tAgeing Timer\n");

	while(cache_iterator_has_next(&iter)) {
		MACTableEntity* entity= cache_iterator_next(&iter);
		Port* port = (Port*)entity->port;

		uint8_t mac[6];
		double ageing = ((double)(entity->timeout - time_ms())) / 1000;
		if(ageing < 0)
			continue;

		for(int i = 0; i < 6; i++) 
			mac[i] = (entity->mac >> (5 - i) * 8) & 0xff;

		printf("\t\t%4d\t%02x:%02x:%02x:%02x:%02x:%02x \t%04f\n", get_portnum(port),
				mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], (double)ageing);
	}
	printf("\n");
}

/* Update entities of MAC filtering table */
static bool update_table(Cache* table, Packet* packet, Port* port) {
	Ether* ether = (Ether*)(packet->buffer + packet->start);

	MACTableEntity* entity = (MACTableEntity*)cache_get(table, (void*)(uint64_t)ether->smac);
	time_t current = time_ms();

	if(!entity) {
		/* Add new entity */
		entity = (MACTableEntity*)malloc(sizeof(MACTableEntity));
		entity->mac = (uint64_t)ether->smac;
		entity->age = 120; // 2 min 
		entity->port = port;
		entity->timeout = current + 1000 * entity->age; 

		if(!cache_set(table, (void*)(uint64_t)ether->smac, (void*)entity)) {
			printf("Cache setting failed\n");
			free(entity);
			return false;
		}
	} else {
		/* Update existed entity */
		entity->timeout = current + 1000 * entity->age;

		CacheIterator iter;
		cache_iterator_init(&iter, table);

		while(cache_iterator_has_next(&iter)) {
			MACTableEntity* entity = cache_iterator_next(&iter);
			if(entity->timeout < current) {
				if(!cache_remove(table, (void*)entity->mac)) {
					printf("Cache remove failed\n");
					return false;
				}
			}
		}
	}
	
	return true;
}

static void get(Node* this) {
	Composite* composite = (Composite*)this;

	printf("\t\t%s\t\t\t   %s\n", composite->name, composite->is_active? "/ON/": "/OFF/"); 
	printf("\t\t-------------------------------\n");
	printf("\t\t");
	for(int i = 0; i < composite->node_count; i++) {
		printf("[%02d] ", i);
	}
	printf("\n");

	printf("\t\t");
	for(int i = 0; i < composite->node_count; i++) {
		Component* component = (Component*)composite->nodes[i];

		if(component->out) 
			printf(" %-4s", component->out->owner->name);
		else
			printf(" --  ");
	}
	printf("\n");
	printf("\n");

	/* Overriding part */
	list_table((EtherSwitch*)composite);
}

static void ether_switch_send(Component* this, Packet* packet) {
	Port* port = (Port*)this;
	if(!port->is_active || !port->owner->is_active) {
		printf("Node %s is inactive\n", port->name); 
		goto failed;
	}

	EtherSwitch* s = (EtherSwitch*)port->owner;

	if(!update_table(s->table, packet, port)) {
		printf("Update MAC table failed\n");
		goto failed;
	}

	Ether* ether = (Ether*)(packet->buffer + packet->start);
	MACTableEntity* entity = (MACTableEntity*)cache_get(s->table, (void*)(uint64_t)ether->dmac);
	if(!entity) {
		if(!switch_broadcast(port, packet)) 
			goto failed;
	} else {
		Port* dst = entity->port;
		if(dst == port) 
			// Do not send myself
			goto failed;

		if(!switch_unicast(dst, packet)) 
			goto failed;
	}

	return;
	
failed:
	free(packet);
}

static void ether_switch_destroy(Node* this) {
	EtherSwitch* s = (EtherSwitch*)this;
	node_unregister(s->name);	

	if(!s->nodes) {
		for(int i = 0; i < s->node_count; i++) {
			Component* component = s->nodes[i];
			if(!component) 
				component->destroy((Node*)component);
		}

		free(s->nodes);
		s->nodes = NULL;
	}

	/* Overriding part */
	if(s->table)
		cache_destroy(s->table);

	free(s);
	s = NULL;
};

Switch* ether_switch_create(int port_count) {
	EtherSwitch* s = malloc(sizeof(EtherSwitch));
	if(!s) 
		return NULL;

	memset(s, 0x0, sizeof(EtherSwitch));

	s->type = NODE_TYPE_ETHER_SWITCH;
	s->node_count = port_count;
	
	/* Inherit */
	if(!composite_inherit((Composite*)s))
		goto failed;

	/* Extends */
	s->table = cache_create(MAC_TABLE_SIZE, free, NULL);
	if(!s->table) 
		goto failed;

	/* Method overriding */
	for(int i = 0; i < s->node_count; i++)
		s->nodes[i]->send = ether_switch_send; 
	s->destroy = ether_switch_destroy;
	s->get = get;

	return (Switch*)s;

failed:
	s->destroy((Node*)s); 
	return NULL;
}

