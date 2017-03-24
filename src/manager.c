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
#include "bridge.h"

static Manager* manager;

bool manager_init() {
	manager = malloc(sizeof(Manager));
	if(!manager)
		return false;

	memset(manager, 0x0, sizeof(Manager));

	/* Event machine init */
	event_init();
#ifdef __LINUX
	/* External interface preparation */
	manager->fds = list_create(NULL);
	if(!manager->fds)
		return false;
#else
	/* for packetngin nic init */
	ni_init();
#endif

	cmd_init();
	input_init();
	bridge_init();

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

#ifndef __LINUX
static char* str[] = { ".0", ".1", ".2", ".3", ".4", ".5", ".6", ".7",
					   ".8", ".9", ".10", ".11", ".12", ".13", ".14", ".15" };
#endif

bool node_register(Composite* node, char* name) {
	for(int i = 0; i < node->node_count; i++) {
#ifdef __LINUX
		sprintf(node->nodes[i]->name, "%s.%d", node->name, i);
#else
		int len = strlen(name);
		strncpy(node->nodes[i]->name, name, 2);
		strncpy(node->nodes[i]->name + len, str[i], 3);
#endif
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

NI* port_attach(EndPointPort* port) {
#ifdef __LINUX
	NI* ni = NULL;
	ni = ni_create(port);
	if(!ni)
		return NULL;

	// NOTE: fd is same as network interface index.
	int fd = ni->ti->fd;
	manager->nis[fd] = ni;
	port->fd = fd;

#else
	NI* ni = NULL;
	for(int i = 0; i < nic_count(); i++) {
		ni = manager->nis[i];
		if(ni->used == 0) {
			port->fd = i;
			ni->nic = nic_get(i);
			ni->used = 1;
			ni->port = port;
			break;
		}
	}
#endif
	return ni;
}

void port_detach(NI* ni) {
#ifdef __LINUX
	ni_destroy(ni);
#else
	ni->used = 0;
	ni->nic = NULL;
#endif
}

