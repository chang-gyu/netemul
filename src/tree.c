#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

static TreeNode* root;
static uint32_t count;

TreeNode* tree_get_root() {
	return root;
}

uint32_t tree_get_count() {
	return count;
}

void tree_init() {
	root = NULL;
	count = 0;
	return;
}

static TreeNode* _add(TreeNode* parent, void* data) {
	TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));

	node->self = data;
	node->parent = parent;
	node->count = 0;

	for(int i = 0; i < MAX_CHILDRUN_COUNT; i++) {
		node->children[i] = NULL;
	}

	return node;
}

TreeNode* tree_add(TreeNode* parent, void* data) {
	TreeNode* node = NULL;

	if(root == NULL && parent == NULL) {
		node = _add(NULL, data);
		root = node;
	} else {
		bool check = false;
		size_t i;
		for(i = 0; i < MAX_CHILDRUN_COUNT; i++) {
			if(parent->children[i] == NULL) {
				check = true;
				break;
			}
		}
		if(check == false) {
			printf("This node is full.\n");
			return NULL;	//fail. Children is full.
		}

		node = _add(parent, data);

		parent->children[i] = node;
		parent->count++;
	}

	count++;
	return node;
}

TreeNode* tree_search(TreeNode* tree, void* data) {
	if(tree == NULL)
		return NULL;

	TreeNode* rtnNode = NULL;

	if(tree->self == data)
		rtnNode = tree;
	else {
		for(int i = 0; i < MAX_CHILDRUN_COUNT; i++) {
			rtnNode = tree_search(tree->children[i], data);
			if(rtnNode)
				break;
		}
	}

	return rtnNode;
}

uint32_t tree_get_children_cnt(TreeNode* tree) {
	uint32_t cnt = 0;
	for(int i = 0; i < MAX_CHILDRUN_COUNT; i++) {
		if(tree->children[i] != NULL) {
			cnt++;
		}
	}
	return cnt;
}

void _destroy(TreeNode* tree) {
	for(int i = 0; i < MAX_CHILDRUN_COUNT; i++) {
		if(tree->children[i] != NULL) {
			_destroy(tree->children[i]);
		}
    }
	if(tree == NULL)
		return;
	count--;
	free(tree);

	return;
}

void tree_destroy(TreeNode* tree) {
	if(tree == NULL)
		return;

	_destroy(tree);
	return;
}

void tree_remove(void* data) {
/* umimplement */
}




