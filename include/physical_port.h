#ifndef __PHYSICAL_PORT_H__
#define __PHYSICAL_PORT_H__

#include "port.h"
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

struct _NI;
typedef struct _NI NI;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    char ifname[16];
    NI*         ni;
    int         fd;
    int         wfd;
    struct sockaddr_ll saddrll;
} PhysicalPort;

Port* physical_port_create();
bool physical_send_port_create(PhysicalPort* port);

#endif /* __PHYSICAL_PORT_H__ */

