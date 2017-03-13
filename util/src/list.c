#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <node.h>

#include <rpc_netemul.h>

int main(int argc, char* argv[]) {
	printf("*** Network Emulator List ***\n");

	int opt;
	uint8_t node_type = NODE_TYPE_NONE;

	if((opt = getopt(argc, argv, "bplsh")) != -1) {
		switch(opt) {
			case 'b':
				node_type = NODE_TYPE_BRIDGE;
				break;
			case 'p':
				node_type = NODE_TYPE_HOST;
				break;
			case 'l':
				node_type = NODE_TYPE_LINK;
				break;
			case 's':
				node_type = NODE_TYPE_ETHER_SWITCH;
				break;
			case 'h':
				node_type = NODE_TYPE_HUB_SWITCH;
				break;
		}
	}

	char* host = getenv("NETEMUL_ADDR");
	char* _port = getenv("NETEMUL_PORT");
	if(!host || !_port) {
		printf("Error: $ source netemul.conf\n");
		exit(-1);
	}
	char* next;
	uint16_t port = strtol(_port, &next, 10);
	//TODO check next

	RPC_NetEmulator* rpc = rpc_netemul_open(host, port, 3); //3sec
	if(!rpc) {
		printf("Error: NetEmulator Connection fail %s:%s\n", host, _port);
		exit(-1);
	}

	bool callback(char* result, void* context) {
		RPC_NetEmulator* rpc = context;
		printf("callback %s\n", result);
		rpc_netemul_close(rpc);
		exit(0);
		return true;
	}

	rpc_list(rpc, node_type, callback, rpc);
	
	while(1) {
		rpc_netemul_loop(rpc);
		if(rpc_netemul_is_closed(rpc))
			break;
	}

	return 0;
}
