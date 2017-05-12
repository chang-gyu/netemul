#include <stdio.h>
#include <string.h>

#include "endpoint.h"
#include "manager.h"
#include "host.h"
#include "physical.h"


EndPoint* endpoint_create(int port_count, int type, void* context) {
	EndPoint* end;
	char* name;
    bool result;

	switch(type) {
        case NODE_TYPE_PHYSICAL:
            if(!(end = physical_create()))
                return NULL;

            name = end->name;
            name[0] = 'p'; // 'pc'

            result = false;
            /* Register endpoint to network emulator manager */
            for(int i = 0; i < MAX_NODE_COUNT; i++) {
                sprintf(&name[1], "%d", i);

                if(!get_node(name)) {
            printf("debug1 : %s\n", name);
                    /* 노드 네임이 결국엔 px.0의 형식인데 이렇게 이름을 등록할지, 아니면 원래 인터페이스 이름으로 추가할지 고민해야함 */
                    if(!node_register((Composite*)end, name))
                        break;

            printf("debug3\n");
                    PhysicalPort* port = (PhysicalPort*)end->nodes[0];
                    strcpy(port->ifname, (char*)context);

                    if(!(port->ni = nic_attach(port)))
                        break;
            printf("debug2\n");

                    if(!(physical_send_port_create(port)))
                        break;

                    result = true;
                    break;
                }

            }
            break;

		case NODE_TYPE_HOST:
			if(!(end = host_create(port_count)))
				return NULL;

			name = end->name;
            name[0] = 'v'; // 'vc'

            result = false;
            /* Register endpoint to network emulator manager */
            for(int i = 0; i < MAX_NODE_COUNT; i++) {
                sprintf(&name[1], "%d", i);

                if(!get_node(name)) {
                    if(!node_register((Composite*)end, name))
                        break;

                    for(int j = 0; j < end->node_count; j++) {
                        VirtualPort* port = (VirtualPort*)end->nodes[j];

                        if(!(port->ni = port_attach(port)))
                            break;
                    }

                    result = true;
                    break;
                }

            }
            break;

        default:
            return NULL;
    }

    if(!result)
        goto failed;

    return end;

failed:
    end->destroy((Node*)end);
    return NULL;
}

