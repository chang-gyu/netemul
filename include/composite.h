#ifndef __COMPOSITE_H__
#define __COMPOSITE_H__

#include "node.h"
#include "component.h"

/* Class Composite implements Node */
typedef struct _composite Composite;
struct _composite {
	Node;

#define MAX_COMPONENT_COUNT	64
	int		node_count;
	Component**	nodes;
};

bool composite_inherit(Composite* composite);

static inline bool is_composite(Node* node) {
	return (node->type >= COMPOSITE_TYPE) ? true : false;
}

#endif /* __COMPOSITE_H__ */
