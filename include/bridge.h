#ifndef __BRIDGE_H__
#define __BRIDGE_H__

#include "endpoint.h"
#include "ni.h"

/* Class Host extends EndPoint */
typedef struct { 
	EndPoint;
} Bridge;

EndPoint* bridge_create(void);

#endif /* __BRIDGE_H__ */
