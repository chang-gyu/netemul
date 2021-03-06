#ifndef __NI_H__
#define __NI_H__

#ifdef NETEMUL_PACKETNGIN
#include <net/nic.h>
#endif

#include "physical_port.h"
#include "virtual_port.h"

typedef struct {
    int		    fd;
    char		name[16];
    uint64_t	mac;
} NI_Context;

/**
 * Network interface.
 */
typedef struct _NI NI;
struct _NI {
    NI_Context* ni_context;
    Port* port;
    int type;
    /* Attached port to this(if not using, NULL) */
//    VirtualPort* vport;
  //  PhysicalPort* pport;
};

NI* ni_create(Port* port, int type);
void ni_destroy(NI* ni);

#endif /* __NI_H__*/
