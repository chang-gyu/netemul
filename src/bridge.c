#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>

#include "bridge.h"
#include "manager.h"

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
