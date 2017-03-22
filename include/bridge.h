#ifndef __BRIDGE_H__
#define __BRIDGE_H__

#include "endpoint.h"
#include "ni.h"

/* Class Host extends EndPoint */
typedef struct { 
	EndPoint;
} Bridge;

typedef struct _b {
	Bridge* bridge;
	char* interface;
} Br;

void bridge_init();
List* bridge_getlist();
void _destroy(char* name);
void bridge_destroy();
bool bridge_attach(Bridge* bridge, char* name);
EndPoint* bridge_create(void);

#endif /* __BRIDGE_H__ */
