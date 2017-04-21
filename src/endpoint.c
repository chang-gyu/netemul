#include <stdio.h>
#include <string.h>

#include "endpoint.h"
#include "manager.h"
#include "host.h"
#include "physical.h"

EndPoint* endpoint_create(int port_count, int type, void* context) {
	EndPoint* end;
	char* name;

	switch(type) {
		case NODE_TYPE_PHYSICAL:
            /*
			if(!(end = physical_create(context)))
				return NULL;
			name = end->name;
			name[0] = 'e'; // 'bc'
			break;
*/
		case NODE_TYPE_HOST:
			if(!(end = host_create(port_count)))
				return NULL;

            printf("debug\n");

			name = end->name;
			name[0] = 'p'; // 'pc'
			break;


		default:
			return NULL;
	}

	bool result = false;
	/* Register endpoint to network emulator manager */

	for(int i = 0; i < MAX_NODE_COUNT; i++) {
		sprintf(&name[1], "%d", i);

		if(!get_node(name)) {
			if(!node_register((Composite*)end, name))
				break;

			for(int j = 0; j < end->node_count; j++) {
				VirtualPort* port = (VirtualPort*)end->nodes[j];

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

