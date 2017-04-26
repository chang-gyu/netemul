#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "link.h"
#include "manager.h"

static void get(Node* this) {
	Link* link = (Link*)this;

	Cable* cable = (Cable*)link->nodes[0];
	Component* src = cable->in;
	Component* dst = cable->out;

	printf("\t\t%s\t\t\t   %s\n", link->name, link->is_active? "/ON/": "/OFF/");
	printf("\t\t-------------------------------\n");
	printf("\t\t%3s -- %3s -- %3s\n\n", (src != NULL)? src->name: "NULL",
			cable->owner->name, (dst != NULL)? dst->name: "NULL");
}

static bool set(Node* this, int argc, char** argv) {	//set l0 latency: 10
	Link* link = (Link*)this;

	Cable* cable = (Cable*)link->nodes[0];
	if(!cable->set((Node*)cable, argc, argv))
		return false;

	cable = (Cable*)link->nodes[1];
	if(!cable->set((Node*)cable, argc, argv))
		return false;
	return true;
}
/**
 * Find out node is available for connecting.
 *
 * @return available component reference or NULL if nothing available.
 */
static Component* is_available(Node* node) {
	if(is_composite(node)) {
		Composite* composite = (Composite*)node;

		for(int i = 0; i < composite->node_count; i++) {
			if(composite->nodes[i]->in == NULL) {
				return composite->nodes[i];
			}
		}

		return NULL;
	} else {
		Component* component = (Component*)node;
		if(!component->out)
			return component;

		return NULL;
	}
}

static Link* _link_create(Node* source, Node* destination) {
	Component* src = is_available(source);
	Component* dst = is_available(destination);

	if(!src) {
		printf("'%s' does not have available ports\n", source->name);
		return NULL;
	}

	if(!dst) {
		printf("'%s' does not have available ports\n", source->name);
		return NULL;
	}

	Link* link = malloc(sizeof(Link));
	if(!link)
		return NULL;

	memset(link, 0x0, sizeof(Link));

	link->type = NODE_TYPE_LINK;
	link->node_count = 2;

	/* Inherit */
	if(!composite_inherit((Composite*)link))
		goto failed;

	/* Extends */
	void connect_cable(Component* source, Component* cable0,
			Component* cable1, Component* destination) {
		/*
		 *		(out)	--> Cable0 --> (in)
		 *  Source					Destination
		 *		(in)	<-- Cable1 <-- (out)
		 */
		source->out = cable0;
		source->in = cable1;

		destination->out = cable1;
		destination->in = cable0;

		cable0->out = destination;
		cable0->in = source;

		cable1->out = source;
		cable1->in = destination;
	}

	connect_cable(src, link->nodes[0], link->nodes[1], dst);

	/* Method overriding */
	link->get = get;
	link->set = set;

	return link;

failed:
	link->destroy((Node*)link);
	return NULL;


}

Link* link_create(Node* source, Node* destination) {
	Link* link = _link_create(source, destination);
	if(!link)
		return NULL;

	char* name = link->name;
	name[0] = 'l';

	/* Register link to network emulator manager */
	bool result = false;

	for(int i = 0; i < MAX_NODE_COUNT; i++) {
		sprintf(&name[1], "%d", i);

		if(!get_node(name)) {
			result = node_register((Composite*)link, name);
			break;

		}
	}

	if(!result) {
		link->destroy((Node*)link);
		return NULL;
	}

	return link;
}

