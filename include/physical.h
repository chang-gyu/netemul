#ifndef __PHYSICAL_PORT_H__
#define __PHYSICAL_PORT_H__

#include "endpoint.h"
#include "ni.h"

/* Class Host extends EndPoint */
typedef struct {
	EndPoint;
} Physical;

typedef struct _b {
	Physical* bridge;
	char* interface;
} Br;

void bridge_init();
List* bridge_getlist();
void _destroy(char* name);
void bridge_destroy();
bool bridge_attach(Physical* bridge, char* name);
EndPoint* bridge_create(void);

#endif /* __PHYSICAL_PORT_H__ */
