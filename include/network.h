#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdbool.h>
#include <net/packet.h>
#include "virtual_port.h"

void network_init();
bool network_process(VirtualPort* port, Packet* packet);

#endif /* __NETWORK_H__ */
