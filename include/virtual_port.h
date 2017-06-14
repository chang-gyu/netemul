#ifndef __VIRTUAL_PORT_H__
#define __VIRTUAL_PORT_H__

#include "port.h"

struct _NI;
typedef struct _NI NI;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    NI*         ni;
    int         fd;
} VirtualPort;

Port* virtual_port_create();

#endif /* __VIRTUAL_PORT_H__ */

