#ifndef __NI_H__
#define __NI_H__

#ifdef NETEMUL_PACKETNGIN
#include <net/nic.h>
#endif

#include "virtual_port.h"

typedef struct {
    int		    fd;
    char		name[16];
    uint64_t	mac;
} TapInterface;

/**
 * Network interface.
 */
typedef struct _NI NI;
struct _NI {
    TapInterface* ti;
    /* Attached port to this */
    union {
        void* port;
//        PhysicalPort* port;
    };
};

NI* ni_create(VirtualPort* port);
//NI* ni_attach(EndPointPort* port);
void ni_destroy(NI* ni);

#endif /* __NI_H__*/
