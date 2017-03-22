#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "composite.h"
#include "manager.h"
#include "cable.h"
#include "bridge.h"

static void destroy(Node* this) {
	Composite* composite = (Composite*)this;
	if(composite->type == NODE_TYPE_BRIDGE) {
			_destroy(composite->name);
	}
	node_unregister(composite->name);	

	if(composite->nodes) {
		// NOTE: node_count decreased in destroy.
		int node_count = composite->node_count;
		for(int i = 0; i < node_count; i++) {
			Component* component = (Component*)composite->nodes[i];
			if(component) {
				component->destroy((Node*)component);
			}
		}

		free(composite->nodes);
		composite->nodes = NULL;
	}

	free(composite);
	composite = NULL;
}

static bool set(Node* this, int argc, char** argv) {
	printf("Set function for '%s' is not implemented\n", this->name);
	return false;
}

static char* get(Node* this) {
	char* result = (char*)malloc(1024);

	Composite* composite = (Composite*)this;

	sprintf(result, "\t\t%s\t\t\t   %s\n", composite->name, composite->is_active? "/ON/": "/OFF/"); 
	sprintf(result, "\t\t-------------------------------\n");
	sprintf(result, "\t\t");
	for(int i = 0; i < composite->node_count; i++) {
		sprintf(result, "[%02d] ", i);
	}
	sprintf(result, "\n");

	sprintf(result, "\t\t");
	for(int i = 0; i < composite->node_count; i++) {
		Component* component = (Component*)composite->nodes[i];

		if(component->out) 
			sprintf(result, " %-4s", component->out->owner->name);
		else
			sprintf(result, " --  ");
	}
	sprintf(result, "\n\n");

	return result;		//TODO malloc size check
}

bool composite_inherit(Composite* composite) {
	composite->is_active = true;
	composite->destroy = destroy;
	composite->set = set;
	composite->get = get; 

	composite->nodes = malloc(sizeof(Node*) * composite->node_count);
	if(!composite->nodes)
		return false;

	memset(composite->nodes, 0x0, sizeof(Node*) * composite->node_count);

	switch(composite->type) {
		case NODE_TYPE_BRIDGE:
		case NODE_TYPE_HOST:
			for(int i = 0; i < composite->node_count; i++) {
				composite->nodes[i] = (Component*)port_create(NODE_TYPE_END_PORT);
				if(!composite->nodes[i])
					return false;

				composite->nodes[i]->owner = composite;
			}
			break;

		case NODE_TYPE_HUB_SWITCH:
		case NODE_TYPE_ETHER_SWITCH:
			for(int i = 0; i < composite->node_count; i++) {
				composite->nodes[i] = (Component*)port_create(NODE_TYPE_PORT);
				if(!composite->nodes[i])
					return false;

				composite->nodes[i]->owner = composite;
			}
			break;

		case NODE_TYPE_LINK:
			for(int i = 0; i < composite->node_count; i++) {
				/**
				 * Cable default attribute.
				 *
				 * Bandwidth	: 1Gbytes.
				 * Error rate	: 0 (No error).
				 * jitter	: 0 (No variance).
				 * latency	: 0 (No delay).
				 */
				composite->nodes[i] = (Component*)cable_create(1000000000, 0, 0, 0);
						
				if(!composite->nodes[i])
					return false;
				
				composite->nodes[i]->owner = composite;
			}
			break;
	}

	return true;
}

