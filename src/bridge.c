#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>
#include <util/list.h>

#include "bridge.h"
#include "manager.h"

static List* bridgeList;

List* bridge_getlist() {
		return bridgeList;
}

void bridge_init() {
		//TODO: physical nic list set.
		if(!bridgeList)
				bridgeList = list_create(NULL);

}

void _destroy(char* name) {
		char command[128] = { 0, };
		sprintf(command, "/sbin/ifconfig %s down", name);
		printf("%s\n", command);
		system(command);
		sprintf(command, "/sbin/brctl delbr %s", name);
		printf("%s\n", command);
		system(command);
}

void bridge_destroy() {
		if(!list_size(bridgeList))
				return;

		ListIterator iter;
		list_iterator_init(&iter, bridgeList);
		while(list_iterator_next(&iter)) {
				Br* br = list_iterator_next(&iter);
				printf("name : %s\n", br->interface);
				_destroy(br->interface);
		}
}


/*
   static void destroy(Node* this) {
   Composite* composite = (Composite*)this;

   if(composite->nodes) {
   for(int i = 0; i < composite->node_count; i++) {
   Port* port = (Port*)composite->nodes[i];
   if(!port) {
// Overriding part 
//				port_detach(port->ni);
port->destroy((Node*)port);
}
}

free(composite->nodes);
composite->nodes = NULL;
}

node_unregister(composite->name);	
free(composite);
composite = NULL;
}
 */

bool bridge_attach(Bridge* bridge, char* name) {
		Br* br = (Br*)malloc(sizeof(Br));
		br->bridge = bridge;
		br->interface = bridge->name;

		if(!list_add(bridgeList, br)) {
				printf("bridge list add failed\n");
				return false;
		}
				printf("bridge list add\n");

		char command[64] = { 0, };
		sprintf(command, "/sbin/brctl addbr %s", bridge->name);
		system(command);


		sprintf(command, "/sbin/brctl addif %s %s.0", bridge->name, bridge->name);
		system(command);

		sprintf(command, "/sbin/ifconfig %s 0", name);
		system(command);

		sprintf(command, "/sbin/brctl addif %s %s", bridge->name, name);
		system(command);

		sprintf(command, "/sbin/ifconfig %s up", bridge->name);
		system(command);

		return true;

}

EndPoint* bridge_create() {
		Bridge* bridge = malloc(sizeof(Bridge));
		if(!bridge) 
				return NULL;

		memset(bridge, 0x0, sizeof(Bridge));

		bridge->type = NODE_TYPE_BRIDGE;
		bridge->node_count = 1;

		/* Inherit */
		if(!composite_inherit((Composite*)bridge))
				goto failed;

		/* Extends */
		// nothing 

		/* Method overriding */
		// nothing 

		return (EndPoint*)bridge;

failed:
		bridge->destroy((Node*)bridge); 
		return NULL;
}
