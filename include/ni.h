#ifndef __NI_H__
#define __NI_H__

#include "end_port.h"

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
    union {
        TapInterface* ti;
        /* TODO: add PacketNgin VNIC here */
    };
    /* Attached port to this */
    EndPort* port;
};	

NI* ni_create(EndPort* port);
void ni_destroy(NI* ni);

#endif /* __NI_H__*/ 
