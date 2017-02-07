#ifndef __ENDPOINT_PORT_H__
#define __ENDPOINT_PORT_H__

#include "port.h"

struct _NI;
typedef struct _NI NI;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    NI*         ni;
    int         fd;
} EndPointPort;

Port* endpoint_port_create();

#endif /* __ENDPOINT_PORT_H__ */

