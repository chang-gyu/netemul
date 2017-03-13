#include <string.h>
#include <strings.h>
#include <malloc.h>
#include <fcntl.h>

#include "node.h"
#include "cable.h"
#include "rpc_netemul.h"

struct {
	uint32_t 	id;
	bool(*callback)(RPC_NetEmulator* rpc, void* context);
	void*		context;
} RPC_NetEmulator_Callback;

#define MAKE_WRITE(TYPE)					\
static int write_##TYPE(RPC_NetEmulator* rpc, TYPE##_t v) {			\
	int len = sizeof(TYPE##_t);				\
	if(rpc->wbuf_index + len > RPC_NETEMUL_BUFFER_SIZE)		\
		return 0;					\
								\
	memcpy(rpc->wbuf + rpc->wbuf_index, &v, len);		\
	rpc->wbuf_index += len;					\
								\
	return len;						\
}

#define MAKE_READ(TYPE)						\
static int read_##TYPE(RPC_NetEmulator* rpc, TYPE##_t* v) {			\
	int len = sizeof(TYPE##_t);				\
	if(rpc->rbuf_read + len > rpc->rbuf_index) {		\
		int len2 = rpc->read(rpc, 			\
			rpc->rbuf + rpc->rbuf_index, 		\
			RPC_NETEMUL_BUFFER_SIZE - rpc->rbuf_index);	\
		if(len2 < 0) {					\
			return len2;				\
		}						\
								\
		rpc->rbuf_index += len2;			\
	}							\
								\
	if(rpc->rbuf_read + len > rpc->rbuf_index)		\
		return 0;					\
								\
	memcpy(v, rpc->rbuf + rpc->rbuf_read, len);		\
	rpc->rbuf_read += len;					\
								\
	return len;						\
}

MAKE_READ(uint8)
MAKE_WRITE(uint8)
MAKE_READ(uint16)
MAKE_WRITE(uint16)
MAKE_READ(uint32)
MAKE_WRITE(uint32)
MAKE_READ(uint64)
MAKE_WRITE(uint64)

MAKE_READ(int32)
MAKE_WRITE(int32)

#define read_bool(RPC_NetEmulator, DATA)	read_uint8((RPC_NetEmulator), (uint8_t*)(DATA))
#define write_bool(RPC_NetEmulator, DATA)	write_uint8((RPC_NetEmulator), (uint8_t)(DATA))

static int write_double(RPC_NetEmulator* rpc, double v) {			
	int len = sizeof(double);				
	if(rpc->wbuf_index + len > RPC_NETEMUL_BUFFER_SIZE)		
		return 0;					
							
	memcpy(rpc->wbuf + rpc->wbuf_index, &v, len);		
	rpc->wbuf_index += len;				
						
	return len;			
}

static int read_double(RPC_NetEmulator* rpc, double* v) {			
	int len = sizeof(double);				
	if(rpc->rbuf_read + len > rpc->rbuf_index) {		
		int len2 = rpc->read(rpc, 			
			rpc->rbuf + rpc->rbuf_index, 		
			RPC_NETEMUL_BUFFER_SIZE - rpc->rbuf_index);	
		if(len2 < 0) {					
			return len2;				
		}						
								
		rpc->rbuf_index += len2;			
	}							
								
	if(rpc->rbuf_read + len > rpc->rbuf_index)		
		return 0;					
								
	memcpy(v, rpc->rbuf + rpc->rbuf_read, len);		
	rpc->rbuf_read += len;					
								
	return len;						
}

static int write_string(RPC_NetEmulator* rpc, const char* v) {
	uint16_t len0;
	if(!v)
		len0 = 0;
	else
		len0 = strlen(v);

	uint16_t len = sizeof(uint16_t) + len0;
	if(rpc->wbuf_index + len > RPC_NETEMUL_BUFFER_SIZE)
		return 0;
	
	memcpy(rpc->wbuf + rpc->wbuf_index, &len0, sizeof(uint16_t));
	memcpy(rpc->wbuf + rpc->wbuf_index + sizeof(uint16_t), v, len0);
	rpc->wbuf_index += len;
	
	return len;
}

static int read_string(RPC_NetEmulator* rpc, char** v, uint16_t* len) {
	*len = 0;
	
	uint16_t len0;
	memcpy(&len0, rpc->rbuf + rpc->rbuf_read, sizeof(uint16_t));
	uint16_t len1 = sizeof(uint16_t) + len0;
	
	if(rpc->rbuf_read + len1 > rpc->rbuf_index) {
		int len2 = rpc->read(rpc, rpc->rbuf + rpc->rbuf_index, 
			RPC_NETEMUL_BUFFER_SIZE - rpc->rbuf_index);
		if(len2 < 0) {
			return len2;
		}
		
		rpc->rbuf_index += len2;
	}
	
	if(rpc->rbuf_read + len1 > rpc->rbuf_index)
		return 0;
	
	*len = len0;
	if(v != NULL)
		*v = (char*)(rpc->rbuf + rpc->rbuf_read + sizeof(uint16_t));
	rpc->rbuf_read += len1;
	
	return len1;
}

static int write_bytes(RPC_NetEmulator* rpc, void* v, int32_t size) {
	int32_t len = sizeof(int32_t) + (size < 0 ? 0 : size);
	if(rpc->wbuf_index + len > RPC_NETEMUL_BUFFER_SIZE)
		return 0;
	
	memcpy(rpc->wbuf + rpc->wbuf_index, &size, sizeof(int32_t));
	if(size > 0)
		memcpy(rpc->wbuf + rpc->wbuf_index + sizeof(int32_t), v, size);
	rpc->wbuf_index += len;
	
	return len;
}

static int read_bytes(RPC_NetEmulator* rpc, void** v, int32_t* len) {
	*len = 0;
	
	int32_t len0;
	memcpy(&len0, rpc->rbuf + rpc->rbuf_read, sizeof(int32_t));
	int32_t len1 = sizeof(int32_t) + (len0 < 0 ? 0 : len0);
	
	if(rpc->rbuf_read + len1 > rpc->rbuf_index) {
		int len2 = rpc->read(rpc, rpc->rbuf + rpc->rbuf_index, 
			RPC_NETEMUL_BUFFER_SIZE - rpc->rbuf_index);
		if(len2 < 0) {
			return len2;
		}
		
		rpc->rbuf_index += len2;
	}
	
	if(rpc->rbuf_read + len1 > rpc->rbuf_index)
		return 0;
	
	*len = len0;
	if(v != NULL) {
		if(len0 > 0) {
			*v = (void*)(rpc->rbuf + rpc->rbuf_read + sizeof(int32_t));
		} else {
			*v = NULL;
		}
	}
	rpc->rbuf_read += len1;
	
	return len1;
}

#define INIT()								\
	__attribute__((__unused__)) int _len = 0;			\
	__attribute__((__unused__)) int _size = 0;			\
	__attribute__((__unused__)) int _rbuf_read = rpc->rbuf_read;	\
	__attribute__((__unused__)) int _wbuf_index = rpc->wbuf_index;

#define ROLLBACK()			\
	rpc->rbuf_read = _rbuf_read;	\
	rpc->wbuf_index = _wbuf_index;
	
#define READ(VALUE)			\
if((_len = (VALUE)) <= 0) {		\
	ROLLBACK();			\
	return _len; 			\
} else {				\
	_size += _len;			\
}

#define READ2(VALUE, FAILED)		\
if((_len = (VALUE)) <= 0) {		\
	ROLLBACK();			\
	FAILED();			\
	return _len; 			\
} else {				\
	_size += _len;			\
}

#define WRITE(VALUE)			\
if((_len = (VALUE)) <= 0) {		\
	ROLLBACK();			\
	return _len; 			\
} else {				\
	_size += _len;			\
}

#define RETURN()	return _size;

#define INIT2()					\
	int _wbuf_index = rpc->wbuf_index;

#define WRITE2(VALUE)			\
if((VALUE) <= 0) {			\
	rpc->wbuf_index = _wbuf_index;	\
	return;				\
}

#define RETURN2()			\
if(rpc->wbuf_index > 0 && rpc->write && wbuf_flush(rpc) < 0 && rpc->close) {	\
	rpc->close(rpc);							\
	return;									\
}

static int rbuf_flush(RPC_NetEmulator* rpc) {
	int len = rpc->rbuf_read;
	memmove(rpc->rbuf, rpc->rbuf + len, rpc->rbuf_index - len);
	
	rpc->rbuf_index -= len;
	rpc->rbuf_read = 0;
	
	return len;
}

static int wbuf_flush(RPC_NetEmulator* rpc) {
	int len = rpc->write(rpc, rpc->wbuf, rpc->wbuf_index);
	if(len < 0) {
		return len;
	}
	memmove(rpc->wbuf, rpc->wbuf + len, rpc->wbuf_index - len);
	
	rpc->wbuf_index -= len;
	
	return len;
}

// API
// hello client API
int rpc_hello(RPC_NetEmulator* rpc, bool(*callback)(void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_HELLO_REQ));
	WRITE(write_string(rpc, RPC_NETEMUL_MAGIC));
	WRITE(write_uint32(rpc, RPC_NETEMUL_VERSION));
	
	rpc->hello_callback = callback;
	rpc->hello_context = context;
	
	RETURN();
}

static int hello_res_handler(RPC_NetEmulator* rpc) {
	if(rpc->hello_callback && !rpc->hello_callback(rpc->hello_context)) {
		rpc->hello_callback = NULL;
		rpc->hello_context = NULL;
	}
	
	return 1;
}

// hello server API
static int hello_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* magic;
	uint16_t len;
	READ(read_string(rpc, &magic, &len));
	if(len != RPC_NETEMUL_MAGIC_SIZE) {
		return -1;
	}
	
	if(memcmp(magic, RPC_NETEMUL_MAGIC, RPC_NETEMUL_MAGIC_SIZE) != 0) {
		return -1;
	}
	
	uint32_t ver;
	READ(read_uint32(rpc, &ver));
	if(ver == RPC_NETEMUL_VERSION) {
		rpc->ver = ver;
	} else {
		return -1;
	}
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_HELLO_RES));
	
	RETURN();
}

// help client API
int rpc_help(RPC_NetEmulator* rpc, bool(*callback)(char* result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_HELP_REQ));
	
	rpc->help_callback = callback;
	rpc->help_context = context;
	
	RETURN();
}

static int help_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* result;
	uint16_t len;

	READ(read_string(rpc, &result, &len));
	
	if(rpc->help_callback && !rpc->help_callback(result, rpc->help_context)) {
		rpc->help_callback = NULL;
		rpc->help_context = NULL;
	}
	
	RETURN();
}

// help server API
void rpc_help_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context) {
	rpc->help_handler = handler;
	rpc->help_handler_context = context;
}
	
static void help_handler_callback(RPC_NetEmulator* rpc, char* result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_HELP_RES));
	WRITE2(write_string(rpc, result));
	
	RETURN2();
}

static int help_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	if(rpc->help_handler) {
		rpc->help_handler(rpc, rpc->help_handler_context, help_handler_callback);
	} else {
		help_handler_callback(rpc, "false");
	}
	
	RETURN();
}

// ifconfig client API
int rpc_ifconfig(RPC_NetEmulator* rpc, bool(*callback)(char* result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_IFCONFIG_REQ));
	
	rpc->ifconfig_callback = callback;
	rpc->ifconfig_context = context;
	
	RETURN();
}

static int ifconfig_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* result;
	uint16_t len;
	//TODO string read
	READ(read_string(rpc, &result, &len));
	
	if(rpc->ifconfig_callback && !rpc->ifconfig_callback(result, rpc->ifconfig_context)) {
		rpc->ifconfig_callback = NULL;
		rpc->ifconfig_context = NULL;
	}
	
	RETURN();
}

// ifconfig server API
void rpc_ifconfig_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context) {
	rpc->ifconfig_handler = handler;
	rpc->ifconfig_handler_context = context;
}

	
static void ifconfig_handler_callback(RPC_NetEmulator* rpc, char* result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_IFCONFIG_RES));
	WRITE2(write_string(rpc, result));
	
	RETURN2();
}

static int ifconfig_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	if(rpc->ifconfig_handler) {
		rpc->ifconfig_handler(rpc, rpc->ifconfig_handler_context, ifconfig_handler_callback);
	} else {
		ifconfig_handler_callback(rpc, false);
	}
	
	RETURN();
}

// tree client API
int rpc_tree(RPC_NetEmulator* rpc, char* node, bool(*callback)(char* result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_TREE_REQ))
	WRITE(write_string(rpc, node));
	
	rpc->tree_callback = callback;
	rpc->tree_context = context;
	
	RETURN();
}

static int tree_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* result;
	uint16_t len;
	//TODO string read
	READ(read_string(rpc, &result, &len));
	
	if(rpc->tree_callback && !rpc->tree_callback(result, rpc->tree_context)) {
		rpc->tree_callback = NULL;
		rpc->tree_context = NULL;
	}
	
	RETURN();
}

// tree server API
void rpc_tree_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context) {
	rpc->tree_handler = handler;
	rpc->tree_handler_context = context;
}

	
static void tree_handler_callback(RPC_NetEmulator* rpc, char* result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_TREE_RES));
	WRITE2(write_string(rpc, result));
	
	RETURN2();
}

static int tree_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	
	if(rpc->tree_handler) {
		rpc->tree_handler(rpc, node, rpc->tree_handler_context, tree_handler_callback);
	} else {
		tree_handler_callback(rpc, false);
	}
	
	RETURN();
}

// list client API
int rpc_list(RPC_NetEmulator* rpc, uint8_t type, bool(*callback)(char* result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_LIST_REQ));
	WRITE(write_uint8(rpc, type));
	
	rpc->list_callback = callback;
	rpc->list_context = context;
	
	RETURN();
}

static int list_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* result;
	uint16_t len;
	//TODO string read
	READ(read_string(rpc, &result, &len));
	
	if(rpc->list_callback && !rpc->list_callback(result, rpc->list_context)) {
		rpc->list_callback = NULL;
		rpc->list_context = NULL;
	}
	
	RETURN();
}

// list server API
void rpc_list_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, uint8_t type, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context) {
	rpc->list_handler = handler;
	rpc->list_handler_context = context;
}

	
static void list_handler_callback(RPC_NetEmulator* rpc, char* result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_LIST_RES));
	WRITE2(write_string(rpc, result));
	
	RETURN2();
}

static int list_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	uint8_t type;
	READ(read_uint8(rpc, &type));
	
	if(rpc->list_handler) {
		rpc->list_handler(rpc, type, rpc->list_handler_context, list_handler_callback);
	} else {
		list_handler_callback(rpc, "false");
	}
	
	RETURN();
}

// create client API
int rpc_create(RPC_NetEmulator* rpc, CreateSpec* spec, bool(*callback)(bool result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_CREATE_REQ));
	WRITE(write_uint8(rpc, spec->type));
	switch(spec->type) {
		case NODE_TYPE_BRIDGE:
			WRITE(write_string(rpc, spec->info.bridge));
			break;
		case NODE_TYPE_HOST:
			WRITE(write_uint32(rpc, spec->info.port_count));
			break;
		case NODE_TYPE_LINK:
			WRITE(write_string(rpc, spec->info.link.node0));
			WRITE(write_string(rpc, spec->info.link.node1));
			break;
		case NODE_TYPE_ETHER_SWITCH:
			WRITE(write_uint32(rpc, spec->info.port_count));
			break;
		case NODE_TYPE_HUB_SWITCH:
			WRITE(write_uint32(rpc, spec->info.port_count));
			break;
	}
	
	rpc->create_callback = callback;
	rpc->create_context = context;
	
	RETURN();
}

static int create_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	bool result;
	READ(read_bool(rpc, &result));
	
	if(rpc->create_callback && !rpc->create_callback(result, rpc->create_context)) {
		rpc->create_callback = NULL;
		rpc->create_context = NULL;
	}
	
	RETURN();
}

// create server API
void rpc_create_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, CreateSpec* spec, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context) {
	rpc->create_handler = handler;
	rpc->create_handler_context = context;
}
	
static void create_handler_callback(RPC_NetEmulator* rpc, bool result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_CREATE_RES));
	WRITE2(write_bool(rpc, result));
	
	RETURN2();
}

static int create_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	CreateSpec spec;
	READ(read_uint8(rpc, &spec.type));
	char* bridge;
	char* node0;
	char* node1;
	uint16_t len;
	switch(spec.type) {
		case NODE_TYPE_BRIDGE:
			READ(read_string(rpc, &bridge, &len));
			strncpy(spec.info.bridge, bridge, IFNAMSIZ);
			break;

		case NODE_TYPE_HOST:
			READ(read_uint32(rpc, &spec.info.port_count));
			break;

		case NODE_TYPE_LINK:
			READ(read_string(rpc, &node0, &len));
			strncpy(spec.info.link.node0, node0, IFNAMSIZ);
			READ(read_string(rpc, &node1, &len));
			strncpy(spec.info.link.node1, node1, IFNAMSIZ);
			break;

		case NODE_TYPE_ETHER_SWITCH:
			READ(read_uint32(rpc, &spec.info.port_count));
			break;

		case NODE_TYPE_HUB_SWITCH:
			READ(read_uint32(rpc, &spec.info.port_count));
			break;
	}
	
	if(rpc->create_handler) {
		rpc->create_handler(rpc, &spec, rpc->create_handler_context, create_handler_callback);
	} else {
		create_handler_callback(rpc, false);
	}
	
	RETURN();
}

// destroy client API
int rpc_destroy(RPC_NetEmulator* rpc, char* node, bool(*callback)(bool result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_DESTROY_REQ));
	WRITE(write_string(rpc, node));
	
	rpc->destroy_callback = callback;
	rpc->destroy_context = context;
	
	RETURN();
}

static int destroy_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	bool result;
	READ(read_bool(rpc, &result));
	
	if(rpc->destroy_callback && !rpc->destroy_callback(result, rpc->destroy_context)) {
		rpc->destroy_callback = NULL;
		rpc->destroy_context = NULL;
	}
	
	RETURN();
}

// destroy server API
void rpc_destroy_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context) {
	rpc->destroy_handler = handler;
	rpc->destroy_handler_context = context;
}

	
static void destroy_handler_callback(RPC_NetEmulator* rpc, bool result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_DESTROY_RES));
	WRITE2(write_bool(rpc, result));
	
	RETURN2();
}

static int destroy_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	
	if(rpc->destroy_handler) {
		rpc->destroy_handler(rpc, node, rpc->destroy_handler_context, destroy_handler_callback);
	} else {
		destroy_handler_callback(rpc, false);
	}
	
	RETURN();
}

// on client API
int rpc_on(RPC_NetEmulator* rpc, char* node, bool(*callback)(bool result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_ON_REQ));
	WRITE(write_string(rpc, node));
	
	rpc->on_callback = callback;
	rpc->on_context = context;
	
	RETURN();
}

static int on_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	bool result;
	READ(read_bool(rpc, &result));
	
	if(rpc->on_callback && !rpc->on_callback(result, rpc->on_context)) {
		rpc->on_callback = NULL;
		rpc->on_context = NULL;
	}
	
	RETURN();
}

// on server API
void rpc_on_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context) {
	rpc->on_handler = handler;
	rpc->on_handler_context = context;
}

	
static void on_handler_callback(RPC_NetEmulator* rpc, bool result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_ON_RES));
	WRITE2(write_bool(rpc, result));
	
	RETURN2();
}

static int on_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	
	if(rpc->on_handler) {
		rpc->on_handler(rpc, node, rpc->on_handler_context, on_handler_callback);
	} else {
		on_handler_callback(rpc, false);
	}
	
	RETURN();
}

// off client API
int rpc_off(RPC_NetEmulator* rpc, char* node, bool(*callback)(bool result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_OFF_REQ));
	WRITE(write_string(rpc, node));
	
	rpc->off_callback = callback;
	rpc->off_context = context;
	
	RETURN();
}

static int off_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	bool result;
	READ(read_bool(rpc, &result));
	
	if(rpc->off_callback && !rpc->off_callback(result, rpc->off_context)) {
		rpc->off_callback = NULL;
		rpc->off_context = NULL;
	}
	
	RETURN();
}

// off server API
void rpc_off_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context) {
	rpc->off_handler = handler;
	rpc->off_handler_context = context;
}

	
static void off_handler_callback(RPC_NetEmulator* rpc, bool result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_OFF_RES));
	WRITE2(write_bool(rpc, result));
	
	RETURN2();
}

static int off_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	
	if(rpc->off_handler) {
		rpc->off_handler(rpc, node, rpc->on_handler_context, on_handler_callback);
	} else {
		off_handler_callback(rpc, false);
	}
	
	RETURN();
}
	
// set client API
int rpc_set(RPC_NetEmulator* rpc, char* node, uint8_t argc, char** argv, bool(*callback)(bool result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_SET_REQ));
	WRITE(write_string(rpc, node));
	WRITE(write_uint8(rpc, argc));
	for(int i = 0; i < argc; i++) {
		WRITE(write_string(rpc, argv[i]));
	}
	
	rpc->set_callback = callback;
	rpc->set_context = context;
	
	RETURN();
}

static int set_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	bool result;
	READ(read_bool(rpc, &result));
	
	if(rpc->set_callback && !rpc->set_callback(result, rpc->set_context)) {
		rpc->set_callback = NULL;
		rpc->set_context = NULL;
	}
	
	RETURN();
}

// set server API
void rpc_set_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, uint8_t argc, char** argv, void* context, void(*callback)(RPC_NetEmulator* rpc, bool result)), void* context) {
	rpc->set_handler = handler;
	rpc->set_handler_context = context;
}
	
static void set_handler_callback(RPC_NetEmulator* rpc, bool result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_SET_RES));
	WRITE2(write_bool(rpc, result));
	
	RETURN2();
}

static int set_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	uint8_t argc;
	READ(read_uint8(rpc, &argc));
	char** argv = calloc(argc, sizeof(char*));
	for(int i = 0; i < argc; i++) {
		char* arg;
		uint16_t len;
		READ(read_string(rpc, &arg, &len));
		argv[i] = strndup(arg, len);
	}
	
	if(rpc->set_handler) {
		rpc->set_handler(rpc, node, argc, argv, rpc->set_handler_context, set_handler_callback);
	} else {
		set_handler_callback(rpc, false);
	}
	//TODO free argv;
	for(int i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);
	
	RETURN();
}


// get client API
int rpc_get(RPC_NetEmulator* rpc, char* node, bool(*callback)(char* result, void* context), void* context) {
	INIT();
	
	WRITE(write_uint16(rpc, RPC_NETEMUL_TYPE_GET_REQ));
	WRITE(write_string(rpc, node));
	
	rpc->get_callback = callback;
	rpc->get_context = context;
	
	RETURN();
}

static int get_res_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* result;
	uint16_t len;
	//TODO string read
	READ(read_string(rpc, &result, &len));
	
	if(rpc->get_callback && !rpc->get_callback(result, rpc->get_context)) {
		rpc->get_callback = NULL;
		rpc->get_context = NULL;
	}
	
	RETURN();
}

// get server API
void rpc_get_handler(RPC_NetEmulator* rpc, void(*handler)(RPC_NetEmulator* rpc, char* node, void* context, void(*callback)(RPC_NetEmulator* rpc, char* result)), void* context) {
	rpc->get_handler = handler;
	rpc->get_handler_context = context;
}

	
static void get_handler_callback(RPC_NetEmulator* rpc, char* result) {
	INIT2();
	
	WRITE2(write_uint16(rpc, RPC_NETEMUL_TYPE_GET_RES));
	WRITE2(write_string(rpc, result));
	
	RETURN2();
}

static int get_req_handler(RPC_NetEmulator* rpc) {
	INIT();
	
	char* node;
	uint16_t len;
	READ(read_string(rpc, &node, &len));
	
	if(rpc->get_handler) {
		rpc->get_handler(rpc, node, rpc->get_handler_context, get_handler_callback);
	} else {
		get_handler_callback(rpc, false);
	}
	
	RETURN();
}




// Handlers
typedef int(*Handler)(RPC_NetEmulator*);

static Handler handlers[] = {
	NULL,
	hello_req_handler,
	hello_res_handler,
	help_req_handler,
	help_res_handler,
	ifconfig_req_handler,
	ifconfig_res_handler,
	tree_req_handler,
	tree_res_handler,
	list_req_handler,
	list_res_handler,
	create_req_handler,
	create_res_handler,
	destroy_req_handler,
	destroy_res_handler,
	on_req_handler,
	on_res_handler,
	off_req_handler,
	off_res_handler,
	set_req_handler,
	set_res_handler,
	get_req_handler,
	get_res_handler,
};

bool rpc_netemul_loop(RPC_NetEmulator* rpc) {
	if(rpc->wbuf_index > 0 && rpc->write) {
		if(wbuf_flush(rpc) < 0 && rpc->close) {
			rpc->close(rpc);
			return false;
		}
	}
	
	bool is_first = true;
	while(true) {
		INIT();
		
		uint16_t type = (uint16_t)-1;
		_len = read_uint16(rpc, &type);
		
		if(_len > 0) {
			if(type >= RPC_NETEMUL_TYPE_END || !handlers[type]) {
				if(rpc->close)
					rpc->close(rpc);
				
				return _size > 0;
			}
		} else if(_len < 0) {
			if(rpc->close)
				rpc->close(rpc);
			
			return _size > 0;
		} else {
			if(!is_first)
				return _size > 0;
		}
		
		if(type != (uint16_t)-1) {
			_len = handlers[type](rpc);
			if(_len > 0) {
				_size += _len;
				
				if(rpc->wbuf_index > 0 && rpc->write && wbuf_flush(rpc) < 0 && rpc->close) {
					rpc->close(rpc);
					return false;
				}
				
				if(rbuf_flush(rpc) < 0 && rpc->close) {
					rpc->close(rpc);
					return false;
				}
			} else if(_len == 0) {
				ROLLBACK();
				return _size > 0;
			} else if(rpc->close) {
				rpc->close(rpc);
				return _size > 0;
			}
		} else {
			return _size > 0;
		}
		
		is_first = false;
	}
}

#ifdef LINUX
#define DEBUG 0
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>

// Posix socket RPC_NetEmulator data
typedef struct {
	int	fd;
	struct	sockaddr_in caddr;
} RPC_NetEmulatorData;

static int sock_read(RPC_NetEmulator* rpc, void* buf, int size) {
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	int len = recv(data->fd, buf, size, MSG_DONTWAIT);
	#if DEBUG
	if(len > 0) {
		for(int i = 0; i < len; i++) {
			printf("%02x ", ((uint8_t*)buf)[i]);
		}
		printf("\n");
	}
	#endif /* DEBUG */
	
	if(len == -1) {
		if(errno == EAGAIN)
			return 0;
				
		return -1;
	} else if(len == 0) {
		return -1;
	} else {
		return len;
	}
}

static int sock_write(RPC_NetEmulator* rpc, void* buf, int size) {
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	int len = send(data->fd, buf, size, MSG_DONTWAIT);
	#if DEBUG
	if(len > 0) {
		for(int i = 0; i < len; i++) {
			printf("%02x ", ((uint8_t*)buf)[i]);
		}
		printf("\n");
	}
	#endif /* DEBUG */
	
	if(len == -1) {
		if(errno == EAGAIN)
			return 0;
				
		return -1;
	} else if(len == 0) {
		return -1;
	} else {
		return len;
	}
}

static void sock_close(RPC_NetEmulator* rpc) {
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	close(data->fd);
	data->fd = -1;
#if DEBUG
	printf("Connection closed : %s\n", inet_ntoa(data->caddr.sin_addr));
#endif
}

RPC_NetEmulator* rpc_netemul_open(const char* host, int port, int timeout) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0)
		return NULL;
	
	void handler(int signo) {
		// Do nothing just interrupt
	}
	
	struct sigaction sigact, old_sigact;
	sigact.sa_handler = handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = SA_INTERRUPT;
	
	if(sigaction(SIGALRM, &sigact, &old_sigact) < 0) {
		close(fd);
		return NULL;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0x0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(host);
	addr.sin_port = htons(port);
	
	alarm(timeout);
	
	if(connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) {
		alarm(0);
		sigaction(SIGALRM, &old_sigact, NULL);
		close(fd);
		return NULL;
	}
	
	alarm(0);
	
	if(sigaction(SIGALRM, &old_sigact, NULL) < 0) {
		close(fd);
		return NULL;
	}
	
	RPC_NetEmulator* rpc = malloc(sizeof(RPC_NetEmulator) + sizeof(RPC_NetEmulatorData));
	rpc->read = sock_read;
	rpc->write = sock_write;
	rpc->close = sock_close;
	
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	data->fd = fd;
	
	return rpc;
}

void rpc_netemul_close(RPC_NetEmulator* rpc) {
	if(rpc->close)
		rpc->close(rpc);
}

RPC_NetEmulator* rpc_netemul_listen(int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd < 0) {
		return NULL;
	}
	
	int reuse = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		perror("Failed to set socket option - SO_REUSEADDR\n");
	
	struct sockaddr_in addr;
	memset(&addr, 0x0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	
	if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return NULL;
	}
	
	RPC_NetEmulator* rpc = malloc(sizeof(RPC_NetEmulator) + sizeof(RPC_NetEmulatorData));
	memset(rpc, 0x0, sizeof(RPC_NetEmulator));
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	data->fd = fd;
	
	return rpc;
}

RPC_NetEmulator* rpc_netemul_accept(RPC_NetEmulator* srpc) {
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)srpc->data;
	if(listen(data->fd, 5) < 0) {
		return NULL;
	}
	
	// TODO: would rather change to nonblock socket
	int rc = fcntl(data->fd, F_SETFL, fcntl(data->fd, F_GETFL, 0) | O_NONBLOCK);
	if(rc < 0)
		perror("Failed to modifiy socket to nonblock\n");
		    
	socklen_t len = sizeof(struct sockaddr_in);
	struct sockaddr_in addr;
	int fd = accept(data->fd, (struct sockaddr*)&addr, &len);
	if(fd < 0)
		return NULL;
	
	RPC_NetEmulator* rpc = malloc(sizeof(RPC_NetEmulator) + sizeof(RPC_NetEmulatorData));
	memcpy(rpc, srpc, sizeof(RPC_NetEmulator));
	rpc->ver = 0;
	rpc->rbuf_read = 0;
	rpc->rbuf_index = 0;
	rpc->wbuf_index = 0;
	rpc->read = sock_read;
	rpc->write = sock_write;
	rpc->close = sock_close;
	
	data = (RPC_NetEmulatorData*)rpc->data;
	memcpy(&data->caddr, &addr, sizeof(struct sockaddr_in));
	data->fd = fd;
	
	return rpc;
}

bool rpc_netemul_is_closed(RPC_NetEmulator* rpc) {
	RPC_NetEmulatorData* data = (RPC_NetEmulatorData*)rpc->data;
	return data->fd < 0;
	
}
#endif /* LINUX */
