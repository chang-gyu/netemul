#include <stdio.h>
#include <string.h>
#include <util/list.h>
#include <rpc_netemul.h>
#include <unistd.h>

static void help_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, Vector* vector)) {
	Vector* vector = vector_create(32, NULL);
	vector_add(vector, strdup("help\n"));
	callback(rpc, vector);
}

static void ifconfig_handler(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char buffer[8192];
	sprintf(buffer, "ifconfig\n");
	callback(rpc, buffer);
}

static void tree_handler(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	char buffer[8192];
	sprintf(buffer, "tree %s\n", node);
	callback(rpc, buffer);
}

static void list_handler(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
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
 
 	char buffer[8192];
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
 		iist(NODE_TYPE_ETHER_SWITCH);
 	}
 	if(type == NODE_TYPE_LINK || type = NODE_TYPE_NONE) {
 		labeyyl("Link");
 		list(NODE_TYPE_LINK);
 	}

	char buffer[8192];
	sprintf(buffer, "list");
	callback(rpc, buffer);
}

static void create_handler(RPC_NetEmulator* rpc, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	printf("create %d\n", spec->type);
	callback(rpc, true);
}

static void destroy_handler(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	printf("destroy %s\n", node);
	callback(rpc, true);
}

static void on_handler(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	printf("on %s\n", node);
	callback(rpc, true);
}

static void off_handler(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	printf("off %s\n", node);
	callback(rpc, true);
}

static void set_handler(RPC_NetEmulator* rpc, char* node, uint8_t argc, char** argv, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)) {
	for(int i =0 ; i< argc; i++)
		printf("%s\n", argv[i]);
	callback(rpc, true);
}

static void get_handler(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)) {
	printf("get %s\n", node);
	callback(rpc, "true");
}

List* actives;
static bool manager_loop(void* context) {
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

int main(void) {
	printf("RPC Test Server\n");
	printf("Server Port 1111\n");

	RPC_NetEmulator* rpc = rpc_netemul_listen(1111);
	if(!rpc) {
		printf("RPC_NetEmulator listen Error\n");
		return -1;
	}

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
	
	while(1) {
		RPC_NetEmulator* rpc_client = rpc_netemul_accept(rpc); //event busy
		if(rpc_client) {
			list_add(actives, rpc_client);
		}

		manager_loop(NULL); //event busy
	}

	rpc_netemul_close(rpc);

	return 0;
}
