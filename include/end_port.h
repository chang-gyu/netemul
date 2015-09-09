#ifndef __END_PORT_H__
#define __END_PORT_H__

#include "port.h"

struct _NI;
typedef struct _NI NI;

/* Class EndPort extends Port */
typedef struct {
    Port;

    /* Attached NI to this */
    NI*         ni;
    int         fd;
} EndPort;

Port* end_port_create();

#endif /* __PORT_H__ */

