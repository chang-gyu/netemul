#include <string.h>
#include <malloc.h>

#include <timer.h>
#include <util/types.h>
#include <util/event.h>
#include <net/nic.h>
#include <net/ether.h>
#include <net/ip.h>

#include "node.h"
#include "component.h"
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
		} else if(strcmp(argv[i], "drop:") == 0) {
			i++;
			if(!is_uint16(argv[i])) {
				printf("Error rate must be double\n");
				return false;
			}
			cable->drop_rate = parse_uint16(argv[i]);
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
		} else if(strcmp(argv[i], "variant:") == 0) {
			i++;
			if(!is_uint64(argv[i])) {
				printf("Latency must be uint64\n");
				return false;
			}
			cable->variant = parse_uint64(argv[i]);
		} else {
			printf("Not supported attribute '%s'\n", argv[i]);
			return false;
		}
	}

	return true;
}

static void get(Node* this) {
	Cable* cable = (Cable*)this;

	printf("Activity   Bandwidth   Latency(Variant)   Jitter   Error Rate   Drop Rate\n");
	printf("=========================================================================\n");
	printf("%8s   %8lu   %6lu(%7lu)   %6.3f   %10.3f	%10.3f\n", 
			cable->is_active? "ON": "OFF", cable->bandwidth, 
			cable->latency, cable->variant, cable->jitter, cable->error_rate, cable->drop_rate); 
}

typedef struct _Cable_Context {
	Cable* cable;
	Packet* packet;
} Cable_Context;

static bool _latency(void* context) {
	Cable_Context* c = (Cable_Context*)context;
	Packet* packet = (Packet*)c->packet;
	Component* this = (Component*)c->cable;

	this->out->send(this->out, packet);

	free(c);
	return false;
}


static void send(Component* this, Packet* packet) {
	/*
	if(!this->is_active || !this->owner->is_active) {
	printf("Node %s is inactive\n", this->name); // check that this node.
	goto failed;
	}   
	if(!this->out) {
		printf("Node %s has no out way\n", this->name);
		goto failed;
	}   
	*/

	Cable* cable = (Cable*)this;

	/* Bandwidth */	

	/* Error rate */
	if(cable->error_rate != 0) {
		if(timer_frequency() % 100 <= cable->error_rate) {
			Ether* ether = (Ether*)(packet->buffer + packet->start);
			IP* ip = (IP*)ether->payload;
			uint16_t ip_body_size = ip->length - ip->ihl * 4;
			uint32_t index = timer_frequency() % ip_body_size;
			
			uint8_t *body;
			body = ((uint8_t*)ip + ip->ihl * 4);

			uint32_t bit_cnt = index % 8;
			uint8_t temp = 0x01;
			for(int i = 0; i < bit_cnt; i++)
				temp *= 2;

			temp = temp ^ body[index];
//			printf("before body[%d]: %02x\n", index, body[index]);
			body[index] = temp;
//			printf("after body[%d]: %02x\n", index, body[index]);
		}
	}

	/* Drop rate */
	if(cable->drop_rate != 0) {
		//	if(drop_cnt <= 0.10 * 100)		//test
		if(timer_frequency() % 100 <= cable->drop_rate)
			goto failed;
	}

	/* Latency(Variant) */
	Cable_Context* context = (Cable_Context*)malloc(sizeof(Cable_Context));
	context->cable = cable;
	context->packet = packet;
	
	uint64_t rate_cnt = 0;
	if(cable->variant != 0)
		rate_cnt = timer_frequency() % cable->variant * 2;
	uint64_t delay = 0;
	if(cable->latency > rate_cnt)
		delay = (timer_frequency() % 2 ? cable->latency - rate_cnt : cable->latency + rate_cnt);
	else {
		delay = (timer_frequency() % 2 ? cable->latency - rate_cnt : cable->latency + rate_cnt);
		if(delay > cable->latency + cable->variant)
			delay = 0;
	}

	printf("delay : %lu\n", delay);
	event_timer_add(_latency, context, delay, 0);
	return;

failed:
#ifdef NETEMUL_PACKETNGIN
	nic_free(packet);
#else
	free(packet);
#endif
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
#ifdef NET_CONTROL
	cable->send = send;
#endif

	return cable;

failed:
	cable->destroy((Node*)cable);
	return NULL;
}
