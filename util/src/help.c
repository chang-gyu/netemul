#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <node.h>

#include <rpc_netemul.h>
#include <util/vector.h>

int main(int argc, char* argv[]) {
	printf("*** Network Emulator Help ***\n");

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

	bool callback(Vector* vector, void* context) {
		RPC_NetEmulator* rpc = context;

		VectorIterator iter;
		vector_iterator_init(&iter, vector);
		while(vector_iterator_has_next(&iter)) {
			char* result = vector_iterator_next(&iter);
			printf("%s", result);
		}
		vector_destroy(vector);

		rpc_netemul_close(rpc);
		exit(0);
		return true;
	}

	rpc_help(rpc, callback, rpc);
	
	while(1) {
		rpc_netemul_loop(rpc);
	}

	return 0;
}
