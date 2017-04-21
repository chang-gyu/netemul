#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <sys/select.h>
#include <util/list.h>
#include <util/map.h>
#include "node.h"
#include "ni.h"
#include "port.h"

/**
 * Network emulator manager.
 */
typedef struct {
#define MAX_NI_COUNT	1024		//packetngin 32
	NI*	nis[MAX_NI_COUNT];
	int nic_count;			///< The number of used nic.

#define MAX_NODE_COUNT	2048		//packetngin 64
	Map*	nodes;			///< All composite of network emulator (Key: name, Value: pointer of node).
	List*	components;		///< All components of network emulator.

#ifdef __LINUX
	List*	fds;			///< Stdin & Tap interface descriptor.
	int	fd_count;
	int	nfds;
	fd_set	read_fds;
#endif
} Manager;

bool manager_init();

/**
 * Create name and register new node to network emulator.
 *
 * Manager is tracking all of composite nodes by name. Component's name is not
 * actaully registered.
 */
bool node_register(Composite* node, char* name);

/**
 * Unregister object from Network Emulator.
 */
void node_unregister(char* name);

/**
 * Get node from Network Emulator by name.
 */
Node* get_node(char* name);
bool fd_add(int fd);
void fd_remove(int fd);
Manager* get_manager();

NI* port_attach(VirtualPort* port);
void port_detach(NI* ni);

#endif /* __MANAGER_H__ */
