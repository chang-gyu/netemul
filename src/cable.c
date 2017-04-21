#include <stdlib.h>		// For strtod, rand
#include <string.h>		// For strcmp, memset
#include <math.h>		// For stdev(sqrt, log, cos, sin)
#include <values.h>		// For stdev(DBL_MIN)

#include <malloc.h>		// For printf

#include <timer.h>
#include <util/types.h>
#include <util/event.h>
#include <net/ether.h>
#include <net/ip.h>

#include "node.h"
#include "component.h"
#include "cable.h"

bool set(Node* this, int argc, char** argv) {
    Cable* cable = (Cable*)this;
    char* flag = NULL;
    double temp;
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
            flag = NULL;
            temp = strtod(argv[i], &flag);
            if(flag[0] != '\0') {
                printf("Error rate must be double\n");
                return false;
            }
            cable->error_rate = temp;
        } else if(strcmp(argv[i], "drop:") == 0) {
            i++;
            flag = NULL;
            temp = strtod(argv[i], &flag);
            if(flag[0] != '\0') {
                printf("Error rate must be double\n");
                return false;
            }
            cable->drop_rate = temp;
        } else if(strcmp(argv[i], "jitter:") == 0) {
            i++;
            if(!is_uint64(argv[i])) {
                printf("Jitter must be uint64\n");
                return false;
            }
            cable->jitter = parse_uint64(argv[i]);
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

static char* get(Node* this) {
    Cable* cable = (Cable*)this;

    char* result = (char*)malloc(512);
    sprintf(result, "Activity   Bandwidth   Latency(Variant)   Error Rate   Drop Rate\n");
    sprintf(result, "%s================================================================\n", result);
    sprintf(result, "%s%8s   %8lu   %6lu(%7lu)  %10.3f	%10.3f\n", result,
            cable->is_active? "ON": "OFF", cable->bandwidth,
            cable->latency, cable->jitter, cable->error_rate, cable->drop_rate);

    return result;
}

typedef struct _Cable_context {
    Cable* cable;
    Packet* packet;
} Cable_context;

static bool _packet_forward(void* context) {
    Cable_context* cable_context = (Cable_context*)context;
    Packet* packet = (Packet*)cable_context->packet;
    Component* this = (Component*)cable_context->cable;

    this->out->packet_forward(this->out, packet);

    free(cable_context);
    return false;
}

static double generateGaussianNoise(double average, double stdev) {
#define EPSILON	DBL_MIN
#define TWO_PI	(2.0 * 3.14159265358979323846)

    static double z1;
    static int generate = 0;

    double z0;

    generate = !generate;

    if(!generate)
        return average + z1 * stdev;

    double u1, u2;
    do {
        u1 = rand() * (1.0 / RAND_MAX);
        u2 = rand() * (1.0 / RAND_MAX);
    } while(u1 <= EPSILON);

    z0 = sqrt(-2.0 * log(u1)) * cos(TWO_PI * u2);
    z1 = sqrt(-2.0 * log(u1)) * sin(TWO_PI * u2);

    return average + z0 * stdev;
}

static void packet_forward(Component* this, Packet* packet) {
    Cable* cable = (Cable*)this;

    /* Bandwidth */
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);
    uint64_t time = (uint64_t)tspec.tv_sec * 1000000 + tspec.tv_nsec / 1000; //ms

    if(cable->output_closed - (1000000 / 1000)/*grace*/ > time) {
        goto failed;
    }

    Ether* ether = (Ether*)packet->buffer;
    IP* ip = (IP*)ether->payload;
    size_t size = (sizeof(Ether) + endian16(ip->length));

    if(cable->output_closed > time)
        cable->output_closed += (long double)1000000 * 8 / cable->bandwidth * size;
    else
        cable->output_closed = time + (long double)1000000 * 8 / cable->bandwidth/*output_wait*/ * size;

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
            body[index] = temp;
        }
    }

    /* Drop rate */
    if(cable->drop_rate != 0) {
        if(timer_frequency() % 100 <= cable->drop_rate)
            goto failed;
    }

    /* Latency & Variant */
    if(cable->latency == 0 && cable->jitter == 0) {
        this->out->packet_forward(this->out, packet);
        return;
    }

    Cable_context* context = (Cable_context*)malloc(sizeof(Cable_context));
    context->cable = cable;
    context->packet = packet;

    /* Variant */
    double delay = cable->latency;
    if(cable->jitter != 0)
        delay = generateGaussianNoise(delay, (double)cable->jitter);

    uint64_t total_delay;
    if(delay < 0)
        total_delay = 0;
    else
        total_delay = (uint64_t)delay;

    event_timer_add(_packet_forward, context, total_delay, 0);

    return;

failed:
    free_func(packet);
}

Cable* cable_create(uint64_t bandwidth, double error_rate, double drop_rate,
        uint64_t jitter, uint64_t latency) {
    Cable* cable = malloc(sizeof(Cable));
    if(!cable)
        return NULL;

    memset(cable, 0x0, sizeof(Cable));

    cable->type = NODE_TYPE_CABLE;

    /* Inherit */
    if(!component_inherit((Component*)cable))
        goto failed;

    /* Extends */
    struct timespec tspec;
    clock_gettime(CLOCK_REALTIME, &tspec);

    cable->output_closed = tspec.tv_sec * 1000000 + tspec.tv_nsec / 1000;
    cable->bandwidth = bandwidth;
    cable->error_rate = error_rate;
    cable->drop_rate = drop_rate;
    cable->jitter = jitter;
    cable->latency = latency;

    /* Method overriding */
    cable->set = set;
    cable->get = get;
    cable->packet_forward = packet_forward;

    return cable;

failed:
    cable->destroy((Node*)cable);
    return NULL;
}
