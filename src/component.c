#include <string.h>
#include <malloc.h>
#include "composite.h"

static void destroy(Node* this) {
	Component* component = (Component*)this;

	Component* src = component->in;
	Component* dst = component->out;

	if(src) 
		src->out = NULL;

	if(dst) 
		dst->in = NULL;

	if(component->owner)
		component->owner->node_count--;

	if(component->queue)
		fifo_destroy(component->queue);

	free(component);
	component = NULL;
}

static bool set(Node* this, int argc, char** argv) {
	printf("Set function for '%s' is not implemented\n", this->name);
	return false;
}

static void get(Node* this) {
	printf("Get function for '%s' is not implemented\n", this->name);
}

static void send(Component* this, Packet* packet) {
	if(!this->is_active || !this->owner->is_active) {
		printf("Node %s is inactive\n", this->name); 
		goto failed;
	}

	if(!this->out) {
		goto failed;
	}

#ifdef NET_CONTROL
	if(!fifo_push(this->out->queue, packet)) {
		goto failed;
	}
#else
	this->out->send(this->out, packet);
#endif

	return;

failed:
	free(packet);
}

bool component_inherit(Component* component) {
	component->is_active = true;
	component->destroy = destroy;
	component->set = set;
	component->get = get; 
	component->send = send;
	if(!(component->queue = fifo_create(PAKCET_QUEUE_SIZE, NULL)))
		return false;

	return true;	
}
