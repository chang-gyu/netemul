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
#include "sketch.h"

static void usage(const char* cmd) {
	printf("\nUsage :\n");
        for(int i = 0; commands[i].name != NULL; i++) {
                if(strcmp(cmd, commands[i].name) == 0) {
			printf("\t%s %s\n\n", cmd, commands[i].args);
                }
        }
}

static int cmd_exit(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
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
            case NODE_TYPE_PHYSICAL:
				list_component("Physical", components);
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
		if((strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "physical") == 0)) {
            label("Physical Devices");
            if(!list(NODE_TYPE_PHYSICAL))
                return -1;

        } else if((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "host") == 0)) {
			label("Virtual Devices");
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
        label("Physical Devices");
        if(!list(NODE_TYPE_PHYSICAL))
            return -1;

        label("Virtual Devices");
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

static int cmd_tree(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
    TreeNode* check_link(Node* node, List* list, TreeNode* parent, char* out_node) {
        char temp[8];
        if(node->type < 100)
            return NULL;

        Composite* _composite = (Composite*)node;

        ListIterator iter;
        list_iterator_init(&iter, list);
        while(list_iterator_has_next(&iter)) {
            Link* link = list_iterator_next(&iter);

            for(int i = 0; i < _composite->node_count; i++) {
                memset(temp, 0, sizeof(temp));
                sprintf(temp, "%s.%d", _composite->name, i);
                for(int j = 0; j < link->node_count; j++) {
                    Cable* cable = (Cable*)link->nodes[j];

                    if(!strcmp(temp, cable->in->name)) {
                        //TreeNode* rtn = tree_add(parent, link);
                        list_remove_data(list, link);
                        strcpy(out_node, cable->out->name);
                        //return rtn;
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
    if(argc != 2)
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
            if(!list_add(composites, node)) {
                list_destroy(composites);
                return -1;
            }
    }

    tree_init();

    if(strlen(argv[1]) > MAX_NAME_LEN) {
        printf("Node '%s' is too long\n", argv[1]);
        return -1;
    }

    Node* node = get_node(argv[1]);
    if(!node) {
        usage(argv[0]);
        printf("Node '%s' does not exist\n", argv[1]);
        list_destroy(composites);
        return -1;
    }

    TreeNode* this = tree_add(tree_get_root(), node);
    check_node(node, composites, this);

    sketch(tree_get_root(), 0, 0);
    printf("%s", sketch_render());
    tree_destroy(tree_get_root());
    list_destroy(composites);

    return 0;
}

static int cmd_create(int argc, char** argv, void(*callback)(char* result, int exit_status)) {
    if(!((argc >= 2) && (argc <= 4))) {
        return CMD_STATUS_WRONG_NUMBER;
    }

    if((strcmp(argv[1], "-p") == 0) || (strcmp(argv[1], "physical") == 0)) {
        // TODO: unimplement
        int port_count = 1;
        EndPoint* physical = endpoint_create(port_count, NODE_TYPE_PHYSICAL, argv[2]);
        if(!physical) {
            printf("Physical create failed\n");
            return -1;
        }

        printf("New physical device '%s' created\n", physical->name);
    } else if((strcmp(argv[1], "-v") == 0) || (strcmp(argv[1], "host") == 0)) {
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

        EndPoint* host = endpoint_create(port_count, NODE_TYPE_HOST, NULL);
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
        .name = "tree",
        .desc = "Show topology of node (Which has the greatest number of node in default)",
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

