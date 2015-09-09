#include <string.h>
#include <malloc.h>
#include "port.h"
#include "end_port.h"

Port* _port_create() {
	Port* port = malloc(sizeof(Port));
	if(!port)
		return NULL;

	memset(port, 0x0, sizeof(Port));

	port->type = NODE_TYPE_PORT;

	/* Inherit */
	if(!component_inherit((Component*)port))
		goto failed;

	/* Extends */ 
	// Nothing

	/* Method overriding */
	// Nothing

	return port;

failed:
	port->destroy((Node*)port);
	return NULL;

}

Port* port_create(int type) {
	switch(type) {
		case NODE_TYPE_PORT:
			return _port_create();
		
		case NODE_TYPE_END_PORT:
			return end_port_create();

		default:
			return NULL;
	}
}
