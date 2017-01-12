#ifndef __ENDPOINT_PORT_H__
#define __ENDPOINT_PORT_H__

#include "port.h"

struct _NIC;
typedef struct _NIC NIC;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    NIC*         nic;
    int         fd;
} EndPointPort;

Port* endpoint_port_create();

#endif /* __ENDPOINT_PORT_H__ */

