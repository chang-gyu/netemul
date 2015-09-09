#ifndef __LINK_H__
#define __LINK_H__

#include "composite.h"
#include "cable.h"

/* Class Link extends Composite */
typedef struct {
	Composite;
} Link;

Link* link_create(Node* source, Node* destination);

#endif /* __LINK_H__ */
