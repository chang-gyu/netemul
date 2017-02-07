#include <string.h>
#include <malloc.h>

#include "host.h"
#include "manager.h"

//static void destroy(Node* this) {
//	Composite* composite = (Composite*)this;
//
//	if(composite->nodes) {
//		for(int i = 0; i < composite->node_count; i++) {
//			Port* port = (Port*)composite->nodes[i];
//			if(!port) {
//				/* Overriding part */
//				port_detach(port->ni);
//				port->destroy((Node*)port);
//			}
//		}
//
//		free(composite->nodes);
//		composite->nodes = NULL;
//	}
//
//	node_unregister(composite->name);	
//	free(composite);
//	composite = NULL;
//}

EndPoint* host_create(int port_count) {
	Host* host = malloc(sizeof(Host));
	if(!host) 
		return NULL;

	memset(host, 0x0, sizeof(Host));

	host->type = NODE_TYPE_HOST;
	host->node_count = port_count;

	/* Inherit */
	if(!composite_inherit((Composite*)host))
		goto failed;

	/* Extends */
	// nothing 
	
	/* Method overriding */
	// nothing 

	printf("host_create success\n");	//debug message
	return (EndPoint*)host;

failed:
	host->destroy((Node*)host); 
	return NULL;
}
