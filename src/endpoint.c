#include <stdio.h>
#include <string.h>

#include "endpoint.h"
#include "manager.h"
#include "host.h"
#include "bridge.h"

EndPoint* endpoint_create(int port_count, int type) {
	EndPoint* end;
	char* name;

	switch(type) {
		case NODE_TYPE_HOST:
			if(!(end = host_create(port_count)))
				return NULL;

			name = end->name;
			name[0] = 'p'; // 'hc'
			break;
		case NODE_TYPE_BRIDGE:
			if(!(end = bridge_create()))  
				return NULL;
			name = end->name;
			name[0] = 'b'; // 'bc'
			break;
			

		default:
			return NULL;
	}

#ifndef __LINUX
#include <net/nic.h>
	Manager* manager = get_manager();
	if(nic_count() < manager->nic_count + port_count)
		goto failed;
#endif
	bool result = false;
	/* Register endpoint to network emulator manager */

	for(int i = 0; i < MAX_NODE_COUNT; i++) {
		sprintf(&name[1], "%d", i);

		if(!get_node(name)) {
			if(!node_register((Composite*)end, name))
				break;

			for(int j = 0; j < end->node_count; j++) {
				EndPointPort* port = (EndPointPort*)end->nodes[j];

				if(!(port->ni = port_attach(port))) 
					break;
			}

			result = true;
			break;
		}
	}

	if(!result) 
		goto failed;

	return end;

failed:
	end->destroy((Node*)end);
	return NULL;
}

