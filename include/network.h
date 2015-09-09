#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdbool.h>
#include <net/packet.h>
#include "end_port.h"

void network_init();
bool network_process(EndPort* port, Packet* packet);

#endif /* __NETWORK_H__ */
