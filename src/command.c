#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <util/cmd.h>
#include <util/list.h>
#include <util/types.h>
#include <util/map.h>

#include "manager.h"
#include "link.h"
#include "switch.h"
#include "endpoint.h"
#include "host.h"
#include "tree.h"

static List* bridgelist;
// Ctrl + C -> needs event handler.

static void usage(const char* cmd) {
	printf("\nUsage :\n");
        for(int i = 0; commands[i].name != NULL; i++) {
                if(strcmp(cmd, commands[i].name) == 0) {
			printf("\t%s %s\n\n", cmd, commands[i].args);
                }
        }
}

static int cmd_exit(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	ListIterator iter;
	list_iterator_init(&iter, bridgelist);
	while(list_iterator_has_next(&iter)) {
		char* name = list_iterator_next(&iter);
		char command[64] = { 0, };
		sprintf(command, "/sbin/ifconfig %s down", name);
		system(command);
		sprintf(command, "/sbin/brctl delbr %s", name);
	}

	printf("Network Emulator Terminated ...\n");
	printf("(Virtual Machines need to be turned off first)\n");
	exit(0);

	return 0;
}

static int cmd_list(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	bool list(int type) {
		void list_component(const char* device_name, List* list) {
			printf("\t%s\n", device_name);
			printf("\t=======================================\n");
			ListIterator iter;
			list_iterator_init(&iter, list);

			while(list_iterator_has_next(&iter)) {
				Node* node = list_iterator_next(&iter);
				node->get(node);
			}
			printf("\n");
		}

		Manager* manager = get_manager();

		MapIterator iter;
		map_iterator_init(&iter, manager->nodes);

		List* components = list_create(NULL);
		if(!components)
			return false;

		while(map_iterator_has_next(&iter)) {
			MapEntry* entry = map_iterator_next(&iter);
			Node* node = (Node*)entry->data;
		
			if(node->type == type)
				if(!list_add(components, node))
					return false;
		}

		switch(type) {
			case NODE_TYPE_BRIDGE:
				list_component("Interface", components);
				break;
			case NODE_TYPE_HOST:
				list_component("Host", components);
				break;
			case NODE_TYPE_HUB_SWITCH:
				list_component("Hub Switch", components);
				break;
			case NODE_TYPE_ETHER_SWITCH:
				list_component("Ether Switch", components);
				break;
			case NODE_TYPE_LINK:
				list_component("Link", components);
				break;
		}

		list_destroy(components);

		return true;
	}

	void label(const char* label) {
		printf("%s\n", label);
		printf("===============================================\n");
	}

	if(!((argc == 1) || (argc == 2)))
		return CMD_STATUS_WRONG_NUMBER;

	
	if(argc == 2) {
		if((strcmp(argv[1], "-b") == 0) || (strcmp(argv[1], "bridge") == 0)) {
			label("Network bridges");
			if(!list(NODE_TYPE_BRIDGE))
				return -1;

		} else if((strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "host") == 0)) {
			label("Endpoint Devices");
			if(!list(NODE_TYPE_HOST))
				return -1;

		} else if((strcmp(argv[1], "-l") == 0) || (strcmp(argv[1], "link") == 0)) {
			label("Link");
			if(!list(NODE_TYPE_LINK))
				return -1;

		} else if((strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "switch") == 0)) {
			label("Ethernet Switch");
			if(!list(NODE_TYPE_ETHER_SWITCH))
				return -1;

		} else if((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "hub") == 0)) {
			label("Hub Switch");
			if(!list(NODE_TYPE_HUB_SWITCH))
				return -1;
		} else {
			usage(argv[0]);
			return CMD_STATUS_NOT_FOUND;
		}
	} else {
		label("Network bridges");
		if(!list(NODE_TYPE_BRIDGE))
			return -1;

		label("Endpoint Devices");
		if(!list(NODE_TYPE_HOST))
			return -1;

		label("Switch");
		if(!list(NODE_TYPE_HUB_SWITCH))
			return -1;

		if(!list(NODE_TYPE_ETHER_SWITCH))
			return -1;

		label("Link");
		if(!list(NODE_TYPE_LINK))
			return -1;

	}


	return 0;
}

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
		children += sketch(node->children[sibling], level + 1, sequence + children);
		
		sibling++;
		if(!node->children[sibling]) {
			return children;
		}
	}
	return children + 1;
}


void render() {
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
	check_rc();

	for(int i = 0; i < row; i++) {
		for(int j = 0; j < column; j++) {
			char pipe[3];
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
			}} else {
				int tmp = strlen(canvas[i][j].node->name);
				if(len > tmp)
					sprintf(temp, "-%s", canvas[i][j].node->name);
				else
					sprintf(temp, "%s", canvas[i][j].node->name);
			}

			printf("%s%s", temp, pipe);
		}
		printf("\n");
	}
	printf("\n");
}

static int cmd_tree(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	TreeNode* check_link(Node* node, List* list, TreeNode* parent, char* out_node) {
		ListIterator iter;
		list_iterator_init(&iter, list);

		char temp[8];
		if(node->type < 100)
			return NULL;

		Composite* _composite = (Composite*)node;

		while(list_iterator_has_next(&iter)) {
			Link* link = list_iterator_next(&iter);

			for(int i = 0; i < _composite->node_count; i++) {
				memset(temp, 0, sizeof(temp));
				sprintf(temp, "%s.%d", _composite->name, i);
				for(int j = 0; j < link->node_count; j++) {
					Cable* cable = (Cable*)link->nodes[j];

					if(!strcmp(temp, cable->in->name)) { 
						//	TreeNode* rtn = tree_add(parent, link);
						list_remove_data(list, link);
						strcpy(out_node, cable->out->name);
						//					return rtn;
						return parent;
					}
				}
			}
		} 
		return NULL;
	}

	char* check_node(Node* node, List* list, TreeNode* parent) {
		Composite* composite = (Composite*)node;
		for(int i = 0; i < composite->node_count; i++) {
			char out_node_name[8];
			TreeNode* rtn = check_link(get_node(node->name), list, parent, out_node_name);
			if(!rtn) {
				break;
			} else {
				char temp[8];	
				memcpy(temp, out_node_name, strlen(out_node_name) + 1);
				strtok(temp, ".");

				Node* out_node = get_node(temp);	
				TreeNode* tree = tree_add(rtn, out_node);
				check_node(out_node, list, tree);
			}
		}
		return NULL;
	}

	if(!((argc == 1) || (argc == 2)))
		return CMD_STATUS_WRONG_NUMBER;

	Manager* manager = get_manager();

	MapIterator iter;
	map_iterator_init(&iter, manager->nodes);	//composite iter.

	List* composites = list_create(NULL);
	if(!composites)
		return false;

	while(map_iterator_has_next(&iter)) {
		MapEntry* entry = map_iterator_next(&iter);
		Node* node = (Node*)entry->data;

		if(node->type == NODE_TYPE_LINK)
			if(!list_add(composites, node))
				return false;
	}

	tree_init();

	if(argc == 1) {


	} else if(argc == 2) {
		Node* node = get_node(argv[1]);
		if(!node) {
			usage(argv[0]);
			printf("Node '%s' does not exist\n", argv[1]);
			return -1;
		}

		TreeNode* this = tree_add(tree_get_root(), node);
		check_node(node, composites, this);

	}

	memset(canvas, 0, sizeof(canvas));
	sketch(tree_get_root(), 0, 0);
	render();
	tree_destroy(tree_get_root());
	return 0;
}

static int cmd_create(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(!((argc >= 2) && (argc <= 4))) { 
		return CMD_STATUS_WRONG_NUMBER;
	}

	if((strcmp(argv[1], "-b") == 0) || (strcmp(argv[1], "bridge") == 0)) {
		EndPoint* bridge = endpoint_create(1, NODE_TYPE_BRIDGE);
		if(!bridge) {
			printf("Phsical create failed\n");
			return -1;
		}

		if(!bridgelist)
			bridgelist = list_create(NULL);

		char command[64] = { 0, };
		sprintf(command, "/sbin/brctl addbr %s", bridge->name);
		system(command);

		list_add(bridgelist, bridge->name);

		sprintf(command, "/sbin/brctl addif %s %s.0", bridge->name, bridge->name);
		system(command);

		sprintf(command, "/sbin/ifconfig %s 0", argv[2]);
		system(command);

		sprintf(command, "/sbin/brctl addif %s %s", bridge->name, argv[2]);
		system(command);

		sprintf(command, "/sbin/ifconfig %s up", bridge->name);
		system(command);

		printf("New network bridge'%s' created.\n", bridge->name);
	} else if((strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "host") == 0)) {
		int port_count = DEFAULT_HOST_PORT_COUNT;   //  default value : 1

		if(argc == 3) {
			if(!is_uint32(argv[2])) {
				printf("Port count must to be number\n");
				return -1;
			}

			port_count = parse_uint32(argv[2]);

			if(port_count > MAX_COMPONENT_COUNT) {
				printf("Cannot make too many ports '%d'\n", port_count);
				return -1;
			}
		}

		EndPoint* host = endpoint_create(port_count, NODE_TYPE_HOST);
		if(!host) {
			printf("Host create failed\n");
			return -1;
		}

		printf("New host device '%s' created\n", host->name);
	} else if((strcmp(argv[1], "-l") == 0) || (strcmp(argv[1], "link") == 0)) {
		if(argc != 4) {
			return CMD_STATUS_WRONG_NUMBER;
		}

		Node* source = get_node(argv[2]);
		Node* destination = get_node(argv[3]);

		if(!source) {
			printf("Node '%s' does not exist\n", argv[2]);
			return -1;
		}

		if(!destination) {
			printf("Node '%s' does not exist\n", argv[3]);
			return -1;
		}

		Link* link = link_create(source, destination);
		if(!link) {
			printf("Link create failed\n");
			return -1;
		}

		printf("New link '%s' created\n", link->name);
	} else if((strcmp(argv[1], "-s") == 0) || (strcmp(argv[1], "switch") == 0)) {
		int port_count = DEFAULT_SWITCH_PORT_COUNT;

		if(argc == 3) {
			if(!is_uint32(argv[2])) {
				printf("Port count must to be number\n");
				return -1;
			}

			port_count = parse_uint32(argv[2]);
			if(port_count > MAX_COMPONENT_COUNT) {
				printf("Cannot make too many ports '%d'\n", port_count);
				return -1;
			}
		}

		Switch* s = switch_create(port_count, NODE_TYPE_ETHER_SWITCH);
		if(!s) {
			printf("Ethernet switch create failed\n");
			return -1;
		}

		printf("New switch '%s' created\n", s->name);
	} else if((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "hub") == 0)) {
		int port_count = DEFAULT_SWITCH_PORT_COUNT;

		if(argc == 3) {
			if(!is_uint32(argv[2])) {
				printf("Port count must to be number\n");
				return -1;
			}

			port_count = parse_uint32(argv[2]);
			if(port_count > MAX_COMPONENT_COUNT) {
				printf("Cannot make too many ports '%d'\n", port_count);
				return -1;
			}
		}

		Switch* s = switch_create(port_count, NODE_TYPE_HUB_SWITCH);
		if(!s) {
			printf("Hub create failed\n");
			return -1;
		} 

		printf("New switch '%s' created\n", s->name);
	} else {
		return CMD_STATUS_NOT_FOUND;
	}

	return 0;
}

static int cmd_destroy(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc < 2)
		return CMD_STATUS_WRONG_NUMBER;

	Node* node = get_node(argv[1]);
	if(!node) {
		usage(argv[0]);
		printf("Node '%s' does not exist\n", argv[1]);
		return -1;
	}

	node->destroy(node);

	printf("Node '%s' destroyed\n", argv[1]);
	return 0;
}

static int cmd_activate(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc < 2)
		return CMD_STATUS_WRONG_NUMBER;

	Node* node = get_node(argv[1]);
	if(!node) {
		usage(argv[0]);
		printf("Node '%s' does not exist\n", argv[1]);
		return -1;
	}

	node->is_active = true;

	printf("Node '%s' activated\n", argv[1]);
	return 0;
}

static int cmd_deactivate(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc < 2)
		return CMD_STATUS_WRONG_NUMBER;

	Node* node = get_node(argv[1]);
	if(!node) {
		usage(argv[0]);
		printf("Node '%s' does not exist\n", argv[1]);
		return -1;
	}

	node->is_active = false;

	printf("Node '%s' deactivated\n", argv[1]);
	return 0;
}

static int cmd_set(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc < 3)
		return CMD_STATUS_WRONG_NUMBER;

	Node* node = get_node(argv[1]);
	if(!node) {
		usage(argv[0]);
		printf("Node '%s' does not exist\n", argv[1]);
		return -1;
	}

	if(!node->set(node, argc - 2, &argv[2])) {
		printf("Attribute of Node '%s' unchanged\n", argv[1]);
		return -1;
	}

	printf("Attribute of Node '%s' changed\n", argv[1]);
	return 0;
}

static int cmd_get(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc < 2)
		return CMD_STATUS_WRONG_NUMBER;

	Node* node = get_node(argv[1]);
	if(!node) {
		usage(argv[0]);
		printf("Node '%s' does not exist\n", argv[1]);
		return -1;
	}

	node->get(node);

	return 0;
}

static int cmd_ifconfig(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
	if(argc != 1)
		return CMD_STATUS_WRONG_NUMBER;

	system("/sbin/ifconfig");
	return 0;
}


Command commands[] = {
	{ 
		.name = "exit",
		.desc = "Exit the CLI",
		.func = cmd_exit
	},
	{ 
		.name = "help",
		.desc = "Show this message",
		.func = cmd_help
	},
	{
		.name = "list",
		.desc = "Show node list", 
		.args = "[NODE_TYPE]",
		.func = cmd_list
	},
	{
		.name = "ifconfig",
		.desc = "Show network interfaces",
		.func = cmd_ifconfig
	},
	{
		.name = "tree",
		.desc = "Show topology of node (which has the greatest number of node in default)",
		.args = "[NODE]",
		.func = cmd_tree
	},
	{
		.name = "create",
		.desc = "Create network node",
		.args = "NODE_TYPE [OPTIONS]",
		.func = cmd_create
	},
	{
		.name = "destroy",
		.desc = "Destroy network node",
		.args = "NODE",
		.func = cmd_destroy
	},
	{
		.name = "on",
		.desc = "Activate network node",
		.args = "NODE",
		.func = cmd_activate
	},
	{
		.name = "off",
		.desc = "Deactivate network node",  
		.args = "NODE",
		.func = cmd_deactivate
	},
	{
		.name = "set",
		.desc = "Set attributes of a node",
		.args = "NODE [ATTRIBUTES]",
		.func = cmd_set
	},
	{
		.name = "get",
		.desc = "Get attributes of a node",
		.args = "NODE",
		.func = cmd_get
	},
	{
		.name = NULL,
	}
};

//not used in packetngin
static int execute_cmd(char* line, bool is_dump) {
	// if is_dump == true then file cmd
	//    is_dump == false then stdin cmd
	if(is_dump == true)
		printf("%s\n", line);

	int exit_status = cmd_exec(line, NULL);

	if(exit_status != 0) {
		if(exit_status == CMD_STATUS_WRONG_NUMBER) {
			printf("wrong number of arguments\n"); 
		} else if(exit_status == CMD_STATUS_NOT_FOUND) {
			printf("wrong name of command\n");
		} else if(exit_status < 0) {
#if DEBUG
			printf("error code : %d\n", exit_status);
#endif
		} else {
			printf("%d'std argument type wrong\n", exit_status); 
		}
	}
	printf("> ");
	fflush(stdout);

	return exit_status;
}

#define MAX_LINE_SIZE		2048


//not used in packetngin
void command_process(int fd) {
	char line[MAX_LINE_SIZE] = {0, };
	char* head;
	int seek = 0;
	int eod = 0; // End of data

	while((eod += read(fd, &line[eod], MAX_LINE_SIZE - eod))) {
		head = line;
		for(; seek < eod; seek++) {
			if(line[seek] == '\n') {
				line[seek] = '\0';
				int ret = execute_cmd(head, fd != STDIN_FILENO);

				if(ret == 0) {
					head = &line[seek] + 1;
				} else { 
					eod = 0;
					return;
				}
			}
		}
		if(head == line && eod == MAX_LINE_SIZE){ // Unfound '\n' and head == 0
			printf("Command line is too long %d > %d\n", eod, MAX_LINE_SIZE);
			eod = 0;
			return;
		} else { // Unfound '\n' and seek != 0
			memmove(line, head, eod - (head - line));
			eod -= head - line;
			seek = eod;
			if(fd == STDIN_FILENO) {
				return;
			} else
				continue;
		}
	}

	if(eod != 0) {
		line[eod] = '\0';
		execute_cmd(&line[0], fd != STDIN_FILENO);
	}
	eod = 0;
}

