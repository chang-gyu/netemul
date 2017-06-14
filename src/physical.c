#include <string.h>
#include <malloc.h>

#include "physical.h"
#include "manager.h"

//static void destroy(Node* this) {
//	Composite* composite = (Composite*)this;
//
//	if(composite->nodes) {
//		for(int i = 0; i < composite->node_count; i++) {
//			Port* port = (Port*)composite->nodes[i];
//			if(!port) {
//				/* Overriding part */
//				port_detach(port->ni);
//				port->destroy((Node*)port);
//			}
//		}
//
//		free(composite->nodes);
//		composite->nodes = NULL;
//	}
//
//	node_unregister(composite->name);
//	free(composite);
//	composite = NULL;
//}

static void get(Node* this) {
	Physical* composite = (Physical*)this;
    PhysicalPort* port = (PhysicalPort*)(composite->nodes[0]);
	printf("\t\t%s\t%s\t\t   %s\n", composite->name, port->ifname, composite->is_active? "/ON/": "/OFF/");
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
	printf("\n\n");
}

EndPoint* physical_create() {
	Physical* physical = malloc(sizeof(Physical));
	if(!physical)
		return NULL;

	memset(physical, 0x0, sizeof(Physical));

	physical->type = NODE_TYPE_PHYSICAL;
	physical->node_count = 1;

	/* Inherit */
	if(!composite_inherit((Composite*)physical))
		goto failed;

	/* Extends */
	// nothing

	/* Method overriding */
	physical->get = get;

	return (EndPoint*)physical;

failed:
	physical->destroy((Node*)physical);
	return NULL;
}
