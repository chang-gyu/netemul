#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <node.h>
#include <net/if.h>

#include <composite.h>
#include <rpc_netemul.h>

int main(int argc, char* argv[]) {
	printf("*** Network Emulator Create ***\n");
	static struct option options[] = {
		{ "bridge", no_argument, 0, 'b' },
		{ "host", no_argument, 0, 'p' },
		{ "link", no_argument, 0, 'l' },
		{ "switch", no_argument, 0, 's' },
		{ "hub", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	int opt;
	int index = 0;
	char* next;
	CreateSpec spec;
	spec.type = NODE_TYPE_NONE;

	while((opt = getopt_long(argc, argv, "b:p:l:s:h:", 
					options, &index)) != -1) {
		switch(opt) {
			case 'b':
				spec.type = NODE_TYPE_BRIDGE;
				strcpy(spec.info.bridge, optarg);
				break;
			case 'p':
				spec.type = NODE_TYPE_HOST;
				spec.info.port_count = strtol(optarg, &next, 10);
				//TODO check next
				if(MAX_COMPONENT_COUNT < spec.info.port_count) {
					printf("Error:\n");
					exit(-1);
				}
				break;
			case 'l':
				spec.type = NODE_TYPE_LINK;
				strncpy(spec.info.link.node0, argv[optind - 1], IFNAMSIZ);
				strncpy(spec.info.link.node1, argv[optind], IFNAMSIZ);
				break;
			case 's':
				spec.type = NODE_TYPE_ETHER_SWITCH;
				spec.info.port_count = strtol(optarg, &next, 10);
				//TODO check next
				if(MAX_COMPONENT_COUNT < spec.info.port_count) {
					printf("Error:\n");
					exit(-1);
				}
				break;
			case 'h':
				spec.type = NODE_TYPE_HUB_SWITCH;
				spec.info.port_count = strtol(optarg, &next, 10);
				//TODO check next
				if(MAX_COMPONENT_COUNT < spec.info.port_count) {
					printf("Error:\n");
					exit(-1);
				}
				break;
		}
	}

	if(spec.type == NODE_TYPE_NONE) { 
		printf("./create [NODE_TYPE[NODE_OPTION]] \n");
		return -1;
	}


	switch(spec.type) {
		case NODE_TYPE_BRIDGE:
			printf("Type Bridge\n");
			printf("Bridge Name: %s\n", spec.info.bridge);
			break;
		case NODE_TYPE_HOST:
			printf("Type Host\n");
			printf("Host Port Count: %d\n", spec.info.port_count);
			break;
		case NODE_TYPE_LINK:
			printf("Type Link\n");
			printf("Link: %s - %s\n", spec.info.link.node0, spec.info.link.node1);
			break;
		case NODE_TYPE_ETHER_SWITCH:
			printf("Type Ether Switch\n");
			printf("Switch Port Count: %d\n", spec.info.port_count);
			break;
		case NODE_TYPE_HUB_SWITCH:
			printf("Type Hub Switch\n");
			printf("Hub Port Count: %d\n", spec.info.port_count);
			break;
	}

	char* host = getenv("NETEMUL_ADDR");
	char* _port = getenv("NETEMUL_PORT");
	if(!host || !_port) {
		printf("Error: $ source netemul.conf\n");
		exit(-1);
	}
	uint16_t port = strtol(_port, &next, 10);
	//TODO check next

	RPC_NetEmulator* rpc = rpc_netemul_open(host, port, 3); //3sec
	if(!rpc) {
		printf("Error: NetEmulator Connection fail %s:%s\n", host, _port);
		exit(-1);
	}

	bool callback(bool result, void* context) {
		//TODO check result
		printf("%s\n", result ? "Create Success" : "Create Fail");
		exit(0);
		return true;
	}

	rpc_create(rpc, &spec, callback, NULL);
	while(1)
		rpc_netemul_loop(rpc);

	return 0;
}
