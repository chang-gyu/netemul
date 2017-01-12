#ifndef __NIC_H__
#define __NIC_H__

#include "endpoint_port.h"

typedef struct {
    int		    fd;
    char		name[16];
    uint64_t	mac;
} TapInterface;

/**
 * Network interface.
 */
typedef struct _NIC NIC;
struct _NIC {
    union {
        TapInterface* ti;
        /* TODO: add PacketNgin VNIC here */
    };
    /* Attached port to this */
    EndPointPort* port;
};	

NIC* nic_create(EndPointPort* port);
void nic_destroy(NIC* nic);

#endif /* __NIC_H__*/ 
