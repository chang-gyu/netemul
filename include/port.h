#ifndef __PORT_H__
#define __PORT_H__

#include "component.h"

#define DEFAULT_HOST_PORT_COUNT		1
#define DEFAULT_SWITCH_PORT_COUNT	4

/* Class Port extends Componenet */
typedef struct {
	Component;
} Port;

Port* port_create(int type, void* context);

#endif /* __PORT_H__ */



