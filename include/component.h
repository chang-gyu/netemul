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
	void		(*send)(Component* this, Packet* packet);
//#define PACKET_QUEUE_SIZE	1024
	FIFO*		queue;
};

bool component_inherit(Component* component); 

#endif /* __COMPONENT_H__ */
