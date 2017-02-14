#ifndef __SWITCH_H__
#define __SWITCH_H__

#include <util/cache.h>
#include "composite.h"
#include "port.h"

/* Abstract Class Switch extends Composite */
typedef struct { 
	Composite;
} Switch;

Switch* switch_create(int port_count, int type);

/**
 * Send packet to specific port.
 *
 * @param port destination port to send.
 * @param packet packet to send.
 * @return true if succeed to send.
 */
bool switch_unicast(Port* port, Packet* packet);

bool switch_multicast(Port* port, Packet* packet);

/**
 * Send packet to all ports attached to the switch.
 *
 * @param source destination port received.
 * @param packet packet to send.
 * @return true if succeed to send.
 */
bool switch_broadcast(Port* port, Packet* packet);

#endif /* __SWITCH_H__ */
