#include <string.h>
#include <malloc.h>
#include <util/types.h>
#include "cable.h"

bool set(Node* this, int argc, char** argv) {
	Cable* cable = (Cable*)this;
	for(int i = 0; i < argc; i++) {
		if(strcmp(argv[i], "band:") == 0) {
			i++;
			if(!is_uint64(argv[i])) {
				printf("Bandwidth must be uint64\n");
				return false;
			}
			cable->bandwidth = parse_uint64(argv[i]);
		} else if(strcmp(argv[i], "error:") == 0) {
			i++;
			if(!is_uint16(argv[i])) {
				printf("Error rate must be double\n");
				return false;
			}
			cable->error_rate = parse_uint16(argv[i]);
		} else if(strcmp(argv[i], "jitter:") == 0) {
			i++;
			if(!is_uint16(argv[i])) {
				printf("Jitter must be uint16\n");
				return false;
			}
			cable->jitter = parse_uint16(argv[i]);
		} else if(strcmp(argv[i], "latency:") == 0) {
			i++;
			if(!is_uint64(argv[i])) {
				printf("Latency must be uint64\n");
				return false;
			}
			cable->latency = parse_uint64(argv[i]);
		} else {
			printf("Not supported attribute '%s'\n", argv[i]);
			return false;
		}
	}

	return true;
}

static void get(Node* this) {
	Cable* cable = (Cable*)this;

	printf("Activity   Bandwidth   Latency   Jitter   Error Rate\n");
	printf("====================================================\n");
	printf("%8s   %8lu   %6lu   %6.3f   %10.3f\n", 
			cable->is_active? "ON": "OFF", cable->bandwidth, 
			cable->latency, cable->jitter, cable->error_rate); 
}

Cable* cable_create(uint64_t bandwidth, double error_rate, double jitter, 
		uint64_t latency) {
	Cable* cable = malloc(sizeof(Cable));
	if(!cable)
		return NULL;

	memset(cable, 0x0, sizeof(Cable));

	cable->type = NODE_TYPE_CABLE;

	/* Inherit */
	if(!component_inherit((Component*)cable))
		goto failed;

	/* Extends */
	cable->bandwidth = bandwidth;
	cable->error_rate = error_rate;
	cable->jitter = jitter;
	cable->latency = latency;

	/* Method overriding */
	cable->set = set;
	cable->get = get;

	return cable;

failed:
	cable->destroy((Node*)cable);
	return NULL;
}
