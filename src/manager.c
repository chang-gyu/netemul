#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <util/map.h>
#include <util/types.h>
#include <util/event.h>
#include <util/cmd.h>

#include "manager.h"
#include "composite.h"
#include "input.h"
#include "network.h"
#include "ni.h"

static Manager* manager;

bool manager_init() {
	manager = malloc(sizeof(Manager));
	if(!manager)
		return false;

	memset(manager, 0x0, sizeof(Manager));

	/* Event machine init */
	event_init();

	/* External interface preparation */
	manager->fds = list_create(NULL);
	if(!manager->fds)
		return false;

	cmd_init();
	input_init();

//	 Internal emulation prepartion
	manager->nodes = map_create(MAX_NODE_COUNT, map_string_hash, map_string_equals, NULL);
	if(!manager->nodes)
		return false;

	manager->components = list_create(NULL);
	if(!manager->components)
		return false;

	network_init();

	return true;
}


bool node_register(Composite* node, char* name) {
	for(int i = 0; i < node->node_count; i++) {
		sprintf(node->nodes[i]->name, "%s.%d", node->name, i);

		if(!list_add(manager->components, node->nodes[i]))
			return false;
	}

	return map_put(manager->nodes, strdup(name), node);
}

void node_unregister(char* name) {
	Composite* node = map_remove(manager->nodes, name);

	for(int i = 0; i < node->node_count; i++)
		list_remove_data(manager->components, node->nodes[i]);
}

Node* get_node(char* name) {
	char temp[MAX_NAME_LEN];
	memcpy(temp, name, strlen(name) + 1);

	strtok(temp, ".");
	char* index_str = strtok(NULL, ".");

	/* Component node get */
	if(index_str) {
		if(!is_uint8(index_str)) {
			printf("Port index must be number\n");
			return NULL;
		}

		Composite* composite = (Composite*)map_get(manager->nodes, temp);
		if(!composite)
			return NULL;

		int index = parse_uint8(index_str);
		if(index >= composite->node_count) {
			printf("Port index '%d' does not exist\n", index);
			return NULL;
		}

		return (Node*)composite->nodes[index];
	/* Composite node get */
	} else {
		return (Node*)map_get(manager->nodes, temp);
	}
}

static void update_fd_info() {
	List* fds = manager->fds;
	manager->nfds = (int64_t)list_get_last(fds) + 1;

	fd_set read_fds;
	FD_ZERO(&read_fds);

	for(int i = 0; i < list_size(fds); i++)
		FD_SET((int64_t)list_get(fds, i), &read_fds);

	manager->read_fds = read_fds;
}

bool fd_add(int fd) {
	if(!list_add(manager->fds, (void*)(int64_t)fd))
		return false;

	/* File descriptor information update */
	manager->fd_count++;
	update_fd_info();

	return true;
}

void fd_remove(int fd) {
	if(!list_remove_data(manager->fds, (void*)(int64_t)fd))
		return;

	/* File descriptor information update */
	manager->fd_count--;
	update_fd_info();
}

Manager* get_manager() {
	return manager;
}

NI* nic_attach(PhysicalPort* port) {
	NI* ni = ni_create(NULL, port);
	if(!ni)
		return NULL;

	// NOTE: fd is same as network interface index.
	int fd = ni->ni_context->fd;
	manager->nis[fd] = ni;
	port->fd = fd;

	return ni;
}


NI* port_attach(VirtualPort* port) {
	NI* ni = ni_create(port, NULL);
	if(!ni)
		return NULL;

	// NOTE: fd is same as network interface index.
	int fd = ni->ni_context->fd;
	manager->nis[fd] = ni;
	port->fd = fd;

	return ni;
}

void port_detach(NI* ni) {
	ni_destroy(ni);
}
