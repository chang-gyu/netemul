#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <node.h>

#include "sketch.h"

typedef struct {
		Node* node;
		bool exsisted;
		int children;
		int pipe;		// head head bridge: 0(-+-), head: 1(---), tail: 2( \-), middle bridge: 3( +-)
} Pixel;
#define ROW 28
#define COLUMN  28

static Pixel canvas[ROW][COLUMN];
static int row;
static int column;

int sketch(TreeNode* node, int level, int sequence) {
		memset(canvas, 0, sizeof(canvas));

		int _sketch(TreeNode* node, int level, int sequence) {
				canvas[sequence][level].node = (Node*)node->self;
				canvas[sequence][level].exsisted = true;
				if(node->count > 0)
						canvas[sequence][level].children = node->count;
				else
						canvas[sequence][level].children = node->count;

				TreeNode* parent = node->parent;
				if(parent != NULL) {
						if(parent->count == 1) 
								canvas[sequence][level].pipe = 1;
						else {
								canvas[sequence][level].pipe = 3;
								if(canvas[sequence][level].node == (Node*)parent->children[parent->count - 1]->self)
										canvas[sequence][level].pipe = 2;
								else if(canvas[sequence][level].node == (Node*)parent->children[0]->self)
										canvas[sequence][level].pipe = 0;
						}
				} 

				int sibling = 0;
				int children = 0;
				while(node->children[sibling]) {
						children += _sketch(node->children[sibling], level + 1, sequence + children);

						sibling++;
						if(!node->children[sibling]) {
								return children;
						}
				}
				return children + 1;
		}

		return _sketch(node, level, sequence);
}


char* sketch_render() {
		void check_rc(void) {
				row = 0;
				column = 0;

				for(int i = 0; i < ROW; i++) {
						for(int j = 0; j < COLUMN -1; j++) {
								if(canvas[i][j].exsisted) {
										if(row < i)
												row = i;
										if(column < j)
												column = j;
								}
						}
				}

				row++;
				column++;
				return;
		}
		Pixel* parent(int row, int column) {
				for(int i = row; i >= 0; i--) {
						if(!canvas[i][column].exsisted)
								continue;
						return &canvas[i][column];
				}
				return NULL;
		}
		bool hasSibling(int row, int column) {
				Pixel* pixel = parent(row, column);
				if(pixel)
						if(pixel->children > 0)
								return true;
				return false;
		}
		int check_blank(int column) {
				int rtn = 0;
				for(int i = 0; i < row; i++) {
						if(canvas[i][column].exsisted) {
								int tmp = strlen(canvas[i][column].node->name);
								if(rtn < tmp)
										rtn = tmp;
						}
				}
				return rtn;
		}

		char* result = (char*)malloc(1024);
		check_rc();

		for(int i = 0; i < row; i++) {
				for(int j = 0; j < column; j++) {
						char pipe[4];
						if(!canvas[i][j + 1].exsisted) {
								strcpy(pipe, "   ");
								if(hasSibling(i, j))
										strcpy(pipe, " | ");
						} else {
								int pipe_flag = canvas[i][j + 1].pipe;
								switch(pipe_flag) {
										case 0:
												strcpy(pipe, "-+-");
												break;
										case 1:
												strcpy(pipe, "---");
												break;
										case 2:
												strcpy(pipe, " +-");
												break;
										case 3:
												strcpy(pipe, " +-");
												break;
										default:
												break;
								}
								Pixel* pixel = parent(i, j);
								if(pixel)
										pixel->children--;
						}
						char temp[8] = { 0, };
						int len = check_blank(j);
						if(!canvas[i][j].exsisted) {
								for(int k = 0; k < len - 1; k++) {
										sprintf(temp, " %s", temp);
								}
						} else {
								int tmp = strlen(canvas[i][j].node->name);
								if(len > tmp)
										sprintf(temp, "-%s", canvas[i][j].node->name);
								else
										sprintf(temp, "%s", canvas[i][j].node->name);
						}

						sprintf(result, "%s%s%s", result, temp, pipe);
				}
				sprintf(result, "%s\n", result);
		}
		sprintf(result, "%s\n", result);

		return strdup(result);
}

