#ifndef __HUB_SWITCH_H__
#define __HUB_SWITCH_H__

#include "switch.h"

/**
 * Ethernet Hub switch. 
 *
 * It broadcasts packets to all ports.
 */ 

/* Class HubSwitch extends Switch */
typedef struct {
	Switch;
} HubSwitch;

Switch* hub_create(int port_count);

#endif /* __HUB_SWITCH_H__ */
