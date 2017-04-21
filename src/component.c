#include <string.h>
#include <malloc.h>
#include "composite.h"

#ifdef __LINUX
void (*free_func)(void*) = free;
#else   // for PacketNgin
void (*free_func)(void*) = nic_free;
#endif

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

static char* get(Node* this) {
	char* result = (char*)malloc(128);
	sprintf(result, "Get function for '%s' is not implemented\n", this->name);

	return result;
}

static void packet_forward(Component* this, Packet* packet) {
	if(!this->is_active || !this->owner->is_active) {
		printf("Node %s is inactive\n", this->name);
		goto failed;
	}

	if(!this->out) {
		printf("Node %s has no out way\n", this->name);
		goto failed;
	}

	if(!fifo_push(this->out->queue, packet))
		goto failed;

	return;

failed:
	free_func(packet);
}

bool component_handler(void* context) {
    Component* component = context;
    FIFO* queue = component->queue;
    Packet* packet;
    while((packet = (Packet*)fifo_pop(queue))) {
        component->packet_forward(component, packet);
        return true;
    }

	return true;
}

bool component_inherit(Component* component) {
	component->is_active = true;
	component->destroy = destroy;
	component->set = set;
	component->get = get;
	component->packet_forward = packet_forward;
	if(!(component->queue = fifo_create(PAKCET_QUEUE_SIZE, NULL)))
		return false;

	event_busy_add(component_handler, component);
	return true;
}

