#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdbool.h>
#include <net/packet.h>
#include "endpoint_port.h"

void network_init();
bool network_process(EndPointPort* port, Packet* packet);

#endif /* __NETWORK_H__ */
