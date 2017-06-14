#ifndef __NE_H__
#define __NE_H__

#include "util/map.h"
#include "composite.h"

/* Class EndPoint extends Composite */
typedef struct {
	Composite;
} EndPoint;

EndPoint* endpoint_create(int port_count, int type, void* context);

#endif /* __NE_H__ */
