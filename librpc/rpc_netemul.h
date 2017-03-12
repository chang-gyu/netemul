#ifndef __CONTROL_RPC_NETEMUL__
#define __CONTROL_RPC_NETEMUL__

#include <util/list.h>
#include <control/vmspec.h>

#include <net/if.h>

#define RPC_NETEMUL_MAGIC		"PNRPC_NETEMUL"
#define RPC_NETEMUL_MAGIC_SIZE		5
#define RPC_NETEMUL_VERSION		1
#define RPC_NETEMUL_BUFFER_SIZE		8192

typedef struct _CreateSpec{
	uint8_t type;
	union {
		uint32_t port_count;
		char bridge[IFNAMSIZ + 1];
		struct _Link {
			char node0[IFNAMSIZ + 1];
			char node1[IFNAMSIZ + 1];
		} link;
	} info;
} CreateSpec;

typedef enum {
	RPC_NETEMUL_TYPE_HELLO_REQ = 1,
	RPC_NETEMUL_TYPE_HELLO_RES,
	RPC_NETEMUL_TYPE_CREATE_REQ,
	RPC_NETEMUL_TYPE_CREATE_RES,
	RPC_NETEMUL_TYPE_ON_REQ,
	RPC_NETEMUL_TYPE_ON_RES,
	RPC_NETEMUL_TYPE_LIST_REQ,
	RPC_NETEMUL_TYPE_LIST_RES,
	RPC_NETEMUL_TYPE_END,
} RPC_NETEMUL_TYPE;

typedef struct _RPC_NetEmulator RPC_NetEmulator;

struct _RPC_NetEmulator {
	// Connection information
	int		ver;
	
	// I/O
	int(*read)(RPC_NetEmulator* rpc, void* buf, int size);
	int(*write)(RPC_NetEmulator* rpc, void* buf, int size);
	void(*close)(RPC_NetEmulator* rpc);
	
	uint8_t		rbuf[RPC_NETEMUL_BUFFER_SIZE];
	int		rbuf_read;
	int		rbuf_index;
	
	uint8_t		wbuf[RPC_NETEMUL_BUFFER_SIZE];
	int		wbuf_index;
	
	// Callbacks
	bool(*hello_callback)(void* context);
	void* hello_context;

	bool(*on_callback)(bool result, void* context);
	void* on_context;
	void(*on_handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result));
	void* on_handler_context;

	bool(*list_callback)(char* result, void* context);
	void* list_context;
	void(*list_handler)(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result));
	void* list_handler_context;

	bool(*create_callback)(bool result, void* context);
	void* create_context;
	void(*create_handler)(RPC_NetEmulator* rpc, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result));
	void* create_handler_context;

	bool(*stdio_callback)(uint16_t written, void* context);
	void* stdio_context;
	void(*stdio_handler)(RPC_NetEmulator* rpc, char* str, uint16_t size, void* context, void(*callback)(RPC_NetEmulator* rpc, uint16_t size));
	void* stdio_handler_context;
	
	// Private data
	uint8_t		data[0];
};

// Client side APIs
#ifdef LINUX
RPC_NetEmulator* rpc_open(const char* host, int port, int timeout);
void rpc_close(RPC_NetEmulator* rpc);
RPC_NetEmulator* rpc_listen(int port);
RPC_NetEmulator* rpc_accept(RPC_NetEmulator* rpc);
bool rpc_is_closed(RPC_NetEmulator* rpc);
#endif /* LINUX */

int rpc_hello(RPC_NetEmulator* rpc, bool(*callback)(void* context), void* context);

int rpc_on(RPC_NetEmulator* rpc, char* node, bool(*callback)(bool result, void* context), void* context);
int rpc_list(RPC_NetEmulator* rpc, uint8_t type, bool(*callback)(char* result, void* context), void* context);
int rpc_create(RPC_NetEmulator* rpc, CreateSpec* spec, bool(*callback)(bool result, void* context), void* context);

// Server side APIs
void rpc_on_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context);
void rpc_list_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context);
void rpc_create_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context);

bool rpc_is_active(RPC_NetEmulator* rpc);
bool rpc_loop(RPC_NetEmulator* rpc);

// Utility
void rpc_vm_dump(VMSpec* vm);
 
#endif /* __CONTROL_RPC_NETEMUL_ */
