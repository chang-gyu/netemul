#ifndef __NI_H__
#define __NI_H__

#ifdef NETEMUL_PACKETNGIN
#include <net/nic.h>
#endif

#include "endpoint_port.h"

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
#ifdef __LINUX
    TapInterface* ti;
#else
	NIC*	nic;
	int		used;
#endif
    /* Attached port to this */
    EndPointPort* port;
};	

#ifndef __LINUX
void ni_init();
#endif

NI* ni_create(EndPointPort* port);
void ni_destroy(NI* ni);

#endif /* __NI_H__*/ 
