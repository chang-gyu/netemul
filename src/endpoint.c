#include <stdio.h>
#include <string.h>

#include <net/nic.h>
#include "endpoint.h"
#include "manager.h"
#include "host.h"


EndPoint* endpoint_create(int port_count, int type) {
	EndPoint* end;
	char* name;

	switch(type) {
		case NODE_TYPE_HOST:
			if(!(end = host_create(port_count)))
				return NULL;

			name = end->name;
			name[0] = 'p'; // 'Pc'
			break;

		default:
			return NULL;
	}
#ifdef __LINUX
	/* Register endpoint to network emulator manager */
	bool result = false;
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
#else
	Manager* manager = get_manager();
	if(nic_count() < manager->nic_count + port_count)
		goto failed;

	// Register endpoint to network emulator manager 
	bool result = false;
	char* st[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15" };  //TODO: change sprintf
	for(int i = 0; i < MAX_NODE_COUNT; i++) {

//		snprintf(&name[1], "%d", i);
		strncpy(name + 1, st[i], 2);

		if(!get_node(name)) {
			if(!node_register((Composite*)end, name))
				break;
			
//			printf("In endpoint_create : node_count : %d\n", end->node_count);
			for(int j = 0; j < end->node_count; j++) {
				EndPointPort* port = (EndPointPort*)end->nodes[j];

				if(!(port->ni = port_attach(port)))
					break;

				
			}

			result = true;
			break;
		}

	}
#endif

	if(!result) 
		goto failed;

//printf("endpoint_create success\n");
	return end;

failed:
	end->destroy((Node*)end);
	return NULL;
}

