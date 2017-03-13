#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/list.h>
#include <rpc_netemul.h>
#include <unistd.h>
#include <util/event.h>
#include <util/cmd.h>
#include <time.h>
#include <malloc.h>

#include "manager.h"
#include "link.h"
#include "switch.h"
#include "endpoint.h"
#include "host.h"
#include "tree.h"
#include "sketch.h"

static void help_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char* stream;
	size_t size;
	FILE* fp = open_memstream(&stream, &size);

        int command_len = 0;
	for(int i = 0; commands[i].name != NULL; i++) {
		int len = strlen(commands[i].name);
		command_len = len > command_len ? len : command_len;
	}

	for(int i = 0; commands[i].name != NULL; i++) {
		// Name
		fprintf(fp, "%s", commands[i].name);
		int len = strlen(commands[i].name);
		len = command_len - len + 2;
		for(int j = 0; j < len; j++)
			fputc(' ', fp);
		
		// Description
		fprintf(fp, "%s\n", commands[i].desc);

		// Arguments
		if(commands[i].args != NULL) {
			for(int j = 0; j < command_len + 2; j++)
				fputc(' ', fp);

			fprintf(fp, "%s\n", commands[i].args);
		}
	}

	fclose(fp);
	callback(rpc, stream);
	free(stream);
}

static void ifconfig_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char buffer[8192] = {0, };
 	FILE* fp = popen("/sbin/ifconfig &", "r");
 	while(fgets(buffer + strlen(buffer), sizeof(buffer) - 1 - strlen(buffer), fp));
	pclose(fp);
	callback(rpc, buffer);
}

static void tree_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
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

	List* composites = list_create(NULL);
	if(!composites) { 
		callback(rpc, "false");
		return;
	}

	Manager* manager = get_manager();
	MapIterator iter;
	map_iterator_init(&iter, manager->nodes);	//composite iter.
	while(map_iterator_has_next(&iter)) {
		MapEntry* entry = map_iterator_next(&iter);
		Node* node = (Node*)entry->data;

		if(node->type == NODE_TYPE_LINK) { 
			if(!list_add(composites, node)) {
				list_destroy(composites);
				callback(rpc, "false");
				return;
			}
		}
	}

	tree_init();

	Node* node = get_node(node_name);
	if(!node) {
		list_destroy(composites);
		callback(rpc, "false");
		return;
	}

	TreeNode* this = tree_add(tree_get_root(), node);
	check_node(node, composites, this);

	sketch(tree_get_root(), 0, 0);

	char* stream;
	size_t size;
	FILE* fp = open_memstream(&stream, &size);
	sketch_render(fp);
	tree_destroy(tree_get_root());
	fclose(fp);
	callback(rpc, stream);
	list_destroy(composites);
	free(stream);
}

static void list_handler(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char* stream;
	size_t size;
	FILE* fp = open_memstream(&stream, &size);

 	bool list(int type) {
 		void list_component(const char* device_name, List* list) {
 			fprintf(fp, "\t%s\n", device_name);
 			fprintf(fp, "\t=======================================\n");
 			ListIterator iter;
 			list_iterator_init(&iter, list);
 
 			while(list_iterator_has_next(&iter)) {
 				Node* node = list_iterator_next(&iter);
 				node->get(node, fp);
 			}
 			fprintf(fp, "\n");
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
 		fprintf(fp, "%s\n", label);
 		fprintf(fp, "===============================================\n");
 	}
 
 	if(type == NODE_TYPE_BRIDGE || type == NODE_TYPE_NONE) {
 		label("Network bridges");
 		list(NODE_TYPE_BRIDGE);
 	}
 	if(type == NODE_TYPE_HOST || type == NODE_TYPE_NONE) {
 		label("Endpoint Devices");
 		list(NODE_TYPE_HOST);
 	}
 	if(type == NODE_TYPE_HUB_SWITCH || type == NODE_TYPE_NONE) {
 		label("Endpoint Devices");
 		list(NODE_TYPE_HUB_SWITCH);
 	}
 	if(type == NODE_TYPE_ETHER_SWITCH || type == NODE_TYPE_NONE) {
 		label("Hub Switch");
 		list(NODE_TYPE_ETHER_SWITCH);
 	}
 	if(type == NODE_TYPE_LINK || type == NODE_TYPE_NONE) {
 		label("Link");
 		list(NODE_TYPE_LINK);
 	}

	fclose(fp);
	callback(rpc, stream);
	free(stream);
}

static void create_handler(RPC_NetEmulator* rpc, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	switch(spec->type) {
		case NODE_TYPE_BRIDGE:
			;
			EndPoint* bridge = endpoint_create(1, NODE_TYPE_BRIDGE);
			if(!bridge) {
				callback(rpc, false);
				return;
			}

			extern List* bridgelist;
			//TODO error check
			if(!bridgelist)
				bridgelist = list_create(NULL);

			char command[64] = { 0, };
			sprintf(command, "/sbin/brctl addbr %s", bridge->name);
			system(command);

			list_add(bridgelist, bridge->name);

			sprintf(command, "/sbin/brctl addif %s %s.0", bridge->name, bridge->name);
			system(command);

			sprintf(command, "/sbin/ifconfig %s 0", spec->info.bridge);
			system(command);

			sprintf(command, "/sbin/brctl addif %s %s", bridge->name, spec->info.bridge);
			system(command);

			sprintf(command, "/sbin/ifconfig %s up", bridge->name);
			system(command);
			break;
		case NODE_TYPE_HOST:
			;
			EndPoint* host = endpoint_create(spec->info.port_count, NODE_TYPE_HOST);
			if(!host) {
				callback(rpc, false);
				return;
			}
			break;
		case NODE_TYPE_LINK:
			;
			Node* source = get_node(spec->info.link.node0);
			Node* destination = get_node(spec->info.link.node1);

			if(!source) {
				callback(rpc, false);
				return;
			}

			if(!destination) {
				callback(rpc, false);
				return;
			}

			Link* link = link_create(source, destination);
			if(!link) {
				callback(rpc, false);
				return;
			}
			break;
		case NODE_TYPE_ETHER_SWITCH:
			;
			Switch* ether_switch = switch_create(spec->info.port_count, NODE_TYPE_ETHER_SWITCH);
			if(!ether_switch) {
				callback(rpc, false);
				return;
			}
			break;
		case NODE_TYPE_HUB_SWITCH:
			;
			Switch* hub_switch = switch_create(spec->info.port_count, NODE_TYPE_HUB_SWITCH);
			if(!hub_switch) {
				callback(rpc, false);
				return;
			} 
			break;

	}

	callback(rpc, true);
}

static void destroy_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	Node* node = get_node(node_name);
	if(!node) {
		callback(rpc, false);
		return;
	}

	node->destroy(node);
	callback(rpc, true);
}

static void on_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	Node* node = get_node(node_name);
	if(!node) {
		callback(rpc, false);
		return;
	}

	node->is_active = true;
	callback(rpc, true);
}

static void off_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	Node* node = get_node(node_name);
	if(!node) {
		callback(rpc, false);
		return;
	}

	node->is_active = false;
	callback(rpc, true);
	return;
}

static void set_handler(RPC_NetEmulator* rpc, char* node_name, uint8_t argc, char** argv, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	Node* node = get_node(node_name);

	if(!node) {
		callback(rpc, false);
		return;
	}

	if(!node->set(node, argc, argv)) {
		callback(rpc, false);
		return;
	}

	callback(rpc, true);
}

static void get_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char* stream;
	size_t size;
	FILE* fp = open_memstream(&stream, &size);

	Node* node = get_node(node_name);
	if(!node) {
		fprintf(fp, "Node does not exist\n");
		fclose(fp);
		callback(rpc, stream);
		free(stream);
		return;
	}

	node->get(node, fp);
	fclose(fp);
	callback(rpc, stream);
	free(stream);
}

static List* actives;
RPC_NetEmulator* rpc;
static bool manager_loop(void* context) {
	RPC_NetEmulator* rpc_client = rpc_netemul_accept(rpc); //event busy
	if(rpc_client) {
		list_add(actives, rpc_client);
	}

	if(!list_is_empty(actives)) {
		ListIterator iter;
		list_iterator_init(&iter, actives);
		while(list_iterator_has_next(&iter)) {
			RPC_NetEmulator* rpc = list_iterator_next(&iter);
			if(!rpc_netemul_is_closed(rpc))
				rpc_netemul_loop(rpc);
			else {
				printf("closed\n");
				list_iterator_remove(&iter);
			}
		}
	}
	
	return true;
}

int rpc_manager_init(void) {
	printf("RPC Test Server\n");
	printf("Server Port 1111\n");
	rpc = rpc_netemul_listen(1111);
	if(!rpc) {
		printf("RPC_NetEmulator listen Error\n");
		return -1;
	}

	//TODO popen occure SIGPIPE
#include <signal.h>
	signal(SIGPIPE, SIG_IGN);
	rpc_help_handler(rpc, help_handler, NULL);
	rpc_ifconfig_handler(rpc, ifconfig_handler, NULL);
	rpc_tree_handler(rpc, tree_handler, NULL);
	rpc_list_handler(rpc, list_handler, NULL);
	rpc_create_handler(rpc, create_handler, NULL);
	rpc_destroy_handler(rpc, destroy_handler, NULL);
	rpc_on_handler(rpc, on_handler, NULL);
	rpc_off_handler(rpc, off_handler, NULL);
	rpc_set_handler(rpc, set_handler, NULL);
	rpc_get_handler( rpc, get_handler, NULL);

	actives = list_create(NULL);
	
	event_busy_add(manager_loop, NULL); //event busy

	return 0;
}

void rpc_manager_destroy(void) {
	rpc_netemul_close(rpc);
}
