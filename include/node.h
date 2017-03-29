#ifndef __NODE_H__
#define __NODE_H__

#include <stdio.h>
#include <stdbool.h>

typedef enum {
	NODE_TYPE_NONE,

	/* Component */
	NODE_TYPE_PORT,
	NODE_TYPE_END_PORT,
	NODE_TYPE_CABLE,

#define COMPOSITE_TYPE	100
	/* Composite */
	NODE_TYPE_BRIDGE = 100,
	NODE_TYPE_HOST,
	NODE_TYPE_LINK,
	NODE_TYPE_HUB_SWITCH,
	NODE_TYPE_ETHER_SWITCH,
	NODE_TYPE_ROUTER, // Not yet

	NODE_TYPE_END
} NodeType;

/* Node Interface */
typedef struct _Node Node;
struct _Node {
	int     type;						    ///< type of node.
#define MAX_NAME_LEN	8
	char	name[MAX_NAME_LEN];				///< name of node.
	bool	is_active;                      ///< current status of node.
	void    (*destroy)(Node* this);			///< destroy function of node itself.
	bool	(*set)(Node* this, int argc, char** argv);
    char*    (*get)(Node* this);
};

#endif /* __NODE_H__ */
