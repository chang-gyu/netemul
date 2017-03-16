#ifndef __SKETCH_H__
#define __SKETCH_H__
#include <stdio.h>
#include <tree.h>

int sketch(TreeNode* node, int level, int sequence);
void sketch_render(FILE* fp);

#endif /*__SKETCH_H__*/
