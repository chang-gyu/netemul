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
#include "bridge.h"

static void help_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, Vector* vector)) {
	Vector* vector = vector_create(128, NULL);
        int command_len = 0;
	for(int i = 0; commands[i].name != NULL; i++) {
		int len = strlen(commands[i].name);
		command_len = len > command_len ? len : command_len;
	}

	for(int i = 0; commands[i].name != NULL; i++) {
		// Name
		char temp[128] = { 0, };
		sprintf(temp, "%s%s", temp, commands[i].name);

		int len = strlen(commands[i].name);
		len = command_len - len + 2;
		for(int j = 0; j < len; j++)
			sprintf(temp, "%s ", temp);

		// Description
		sprintf(temp, "%s%s\n", temp, commands[i].desc);
		bool t = vector_add(vector, strdup(temp));
		if(!t)
			printf("helelfe\n");
		
		// Arguments
		if(commands[i].args != NULL) {
			memset(temp, 0, sizeof(temp));
			for(int j = 0; j < command_len + 2; j++)
				sprintf(temp, "%s ", temp);

			sprintf(temp, "%s%s\n", temp, commands[i].args);

			t = vector_add(vector, strdup(temp));
			if(!t)
				printf("fdf\n");
		}
	}

	callback(rpc, vector);

	vector_destroy(vector);
}

static void ifconfig_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char buffer[8192] = {0, };
	FILE* fp = popen("/sbin/ifconfig &", "r");
	while(fgets(buffer + strlen(buffer), sizeof(buffer) - 1 - strlen(buffer), fp));
	pclose(fp);
	callback(rpc, buffer);
}

static void tree_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, Vector* vector)) {
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

	Vector* vector = vector_create(128, NULL);

	List* composites = list_create(NULL);
	if(!composites) { 
		callback(rpc, NULL);	//TODO
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
				callback(rpc, NULL);
				return;
			}
		}
	}

	tree_init();

	Node* node = get_node(node_name);
	if(!node) {
		list_destroy(composites);
		callback(rpc, NULL);
		return;
	}

	TreeNode* this = tree_add(tree_get_root(), node);
	check_node(node, composites, this);

	sketch(tree_get_root(), 0, 0);

	char* result = sketch_render();
	vector_add(vector, result);
	tree_destroy(tree_get_root());
	callback(rpc, vector);
	list_destroy(composites);
}

static void list_handler(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, Vector* vector)) {
	char* list(int type) {
		char* list_component(const char* device_name, List* list) {
			char temp[1024] = { 0, };

			sprintf(temp, "%s\t%s\n", temp, device_name);
			sprintf(temp, "%s\t=======================================\n", temp);
			ListIterator iter;
			list_iterator_init(&iter, list);

			while(list_iterator_has_next(&iter)) {
				Node* node = list_iterator_next(&iter);
				sprintf(temp, "%s%s\n", temp, node->get(node));
			}
			sprintf(temp, "%s\n", temp);

			return strdup(temp);
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

		char* result = NULL;

		switch(type) {
			case NODE_TYPE_BRIDGE:
				result = list_component("Bridge", components);
				break;
			case NODE_TYPE_HOST:
				result = list_component("Host", components);
				break;
			case NODE_TYPE_HUB_SWITCH:
				result = list_component("Hub Switch", components);
				break;
			case NODE_TYPE_ETHER_SWITCH:
				result = list_component("Ether Switch", components);
				break;
			case NODE_TYPE_LINK:
				result = list_component("Link", components);
				break;
		}

		list_destroy(components);

		return result;
	}

	char* label(const char* label) {
		char temp[128] = { 0, };
		sprintf(temp, "%s\n", label);
		sprintf(temp, "%s===============================================\n", temp);

		return strdup(temp);
	}

	Vector* vector = vector_create(512, NULL);
	char* result = NULL;

	if(type == NODE_TYPE_BRIDGE || type == NODE_TYPE_NONE) {
		result = label("Network bridges");
		vector_add(vector, result);
		result = list(NODE_TYPE_BRIDGE);
		vector_add(vector, result);
	}
	if(type == NODE_TYPE_HOST || type == NODE_TYPE_NONE) {
		result = label("Endpoint Devices");
		vector_add(vector, result);
		result = list(NODE_TYPE_HOST);
		vector_add(vector, result);
	}
	if(type == NODE_TYPE_HUB_SWITCH || type == NODE_TYPE_NONE) {
		result = label("Switch");
		vector_add(vector, result);
		result = list(NODE_TYPE_HUB_SWITCH);
		vector_add(vector, result);
	}
	if(type == NODE_TYPE_ETHER_SWITCH || type == NODE_TYPE_NONE) {
		if(type != NODE_TYPE_NONE) {
			result = label("Switch");
			vector_add(vector, result);
		}
		result = list(NODE_TYPE_ETHER_SWITCH);
		vector_add(vector, result);
	}
	if(type == NODE_TYPE_LINK || type == NODE_TYPE_NONE) {
		result = label("Link");
		vector_add(vector, result);
		result = list(NODE_TYPE_LINK);
		vector_add(vector, result);
	}

	callback(rpc, vector);
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

			List* bridgelist = bridge_getlist();
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

static void get_handler(RPC_NetEmulator* rpc, char* node_name, void* context, void(*callback)(RPC_NetEmulator* rpc, Vector* vector)) {
	Vector* vector = vector_create(64, NULL);

	Node* node = get_node(node_name);
	if(!node) {
		vector_add(vector, strdup("Node does not exist\n"));
		callback(rpc, vector);
		return;
	}

	vector_add(vector, node->get(node));
	callback(rpc, vector);
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
	rpc_get_handler(rpc, get_handler, NULL);

	actives = list_create(NULL);

	event_busy_add(manager_loop, NULL); //event busy

	return 0;
}

void rpc_manager_destroy(void) {
	rpc_netemul_close(rpc);
}
