#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <net/packet.h>
#include <util/fifo.h>
#include "node.h"

struct _composite;
typedef struct _composite Composite;

/* Class Component implements Node */
typedef struct _Component Component;
struct _Component{
	Node;

	Composite*	owner;
	Component*	in;
	Component*	out;
	void		(*packet_forward)(Component* this, Packet* packet);
    Packet*     (*packet_read)(Component* this);
#define PAKCET_QUEUE_SIZE	1024
	FIFO*		queue;
};

bool component_inherit(Component* component);


void (*free_func)(void*);

#endif /* __COMPONENT_H__ */
