#ifndef __PHYSICAL_H__
#define __PHYSICAL_H__

#include "endpoint.h"
#include "ni.h"

/* Class Host extends EndPoint */
typedef struct {
	EndPoint;
} Physical;

EndPoint* physical_create();

#endif /* __PHYSICAL_H__ */
