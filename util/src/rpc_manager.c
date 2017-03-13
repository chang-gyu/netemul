#include <stdio.h>
#include <util/list.h>
#include <rpc_netemul.h>
#include <unistd.h>

static void list_handler(RPC_NetEmulator* rpc_netemul, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc_netemul, char* result)) {
	//TODO
	printf("list %d\n", type);
	callback(rpc_netemul, "ASDFASDF");
}

static void create_handler(RPC_NetEmulator* rpc_netemul, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc_netemul, bool result)) {
	//TODO
	printf("create\n");
	callback(rpc_netemul, true);
}

static void on_handler(RPC_NetEmulator* rpc_netemul, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc_netemul, bool result)) {
	Node* node = get_node(node);
	if(!node) {
		printf("Node '%s' does not exist\n", node);
		callback(rpc_netemul, false);
		return;
	}

	node->is_active = true;
	callback(rpc_netemul, true);
}

List* actives;

static bool manager_loop(void* context) {
	if(!list_is_empty(actives)) {
		ListIterator iter;
		list_iterator_init(&iter, actives);
		while(list_iterator_has_next(&iter)) {
			RPC_NetEmulator* rpc_netemul = list_iterator_next(&iter);
			if(!rpc_netemul_is_closed(rpc_netemul))
				rpc_netemul_loop(rpc_netemul);
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

	RPC_NetEmulator* rpc_netemul = rpc_netemul_listen(1111);
	if(!rpc_netemul) {
		printf("RPC_NetEmulator listen Error\n");
		return -1;
	}

	rpc_list_handler(rpc_netemul, list_handler, NULL);
	rpc_create_handler(rpc_netemul, create_handler, NULL);
	rpc_on_handler(rpc_netemul, on_handler, NULL);

	actives = list_create(NULL);
	
	while(1) {
		RPC_NetEmulator* rpc_client = rpc_netemul_accept(rpc_netemul); //event busy
		if(rpc_client) {
			list_add(actives, rpc_client);
		}

		manager_loop(NULL); //event busy
	}

	rpc_netemul_close(rpc_netemul);

	return 0;
}
