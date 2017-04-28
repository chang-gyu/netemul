#ifndef __PHYSICAL_PORT_H__
#define __PHYSICAL_PORT_H__

#include "port.h"

struct _NI;
typedef struct _NI NI;

/* Class EndPointPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    NI*         ni;
    int         fd;
} PhysicalPort;

Port* physical_port_create();

#endif /* __PHYSICAL_PORT_H__ */

