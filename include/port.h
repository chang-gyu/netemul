#ifndef __PORT_H__
#define __PORT_H__

#include "component.h"

#define DEFAULT_HOST_PORT_COUNT		4
#define DEFAULT_SWITCH_PORT_COUNT	16

/* Class Port extends Componenet */
typedef struct {
	Component;
} Port;

Port* port_create(int type);

#endif /* __PORT_H__ */



