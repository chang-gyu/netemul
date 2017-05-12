#ifndef __PHYSICAL_PORT_H__
#define __PHYSICAL_PORT_H__

#include "port.h"

struct _NI;
typedef struct _NI NI;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    char ifname[16];
    /* Attached NI to this */
    NI*         ni;
    int         fd;
    int         wfd;
    struct sockaddr_ll* saddrll;
} PhysicalPort;

Port* physical_port_create();

#endif /* __PHYSICAL_PORT_H__ */

