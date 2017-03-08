#ifndef __TREE_H__
#define __TREE_H__

#include "composite.h"		//MAX_COMPONENT_COUNT
#define MAX_CHILDRUN_COUNT MAX_COMPONENT_COUNT
typedef struct _TreeNode TreeNode;
struct _TreeNode {
	void* self;
	TreeNode* parent;
	TreeNode* children[MAX_CHILDRUN_COUNT];
	int count;
}; 

#endif /* __TREE_H__ */


TreeNode* tree_get_root();
uint32_t tree_get_count();
void tree_init();
TreeNode* tree_add(TreeNode* parent, void* data);
TreeNode* tree_search(TreeNode* tree, void* data);
uint32_t tree_get_children_cnt(TreeNode* node); 
void tree_destroy(TreeNode* this);

