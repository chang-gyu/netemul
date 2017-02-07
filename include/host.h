#ifndef __HOST_H__
#define __HOST_H__

#include "endpoint.h"
#include "ni.h"

/* Class Host extends EndPoint */
typedef struct { 
	EndPoint;
} Host;

EndPoint* host_create(int port_count);

#endif /* __HOST_H__ */
