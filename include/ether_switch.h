#ifndef __ETHER_SWITCH_H__
#define __ETHER_SWITCH_H__

#include "switch.h"

/**
 * Ethernet switch. 
 *
 * It switches packets using MAC filtering table.
 */ 

/* Class EtherSwitch extends Switch */
typedef struct {
	Switch;

#define MAC_TABLE_SIZE	32
	Cache* 	table;
} EtherSwitch;

Switch* ether_switch_create(int port_count);

#endif /* __ETHER_SWITCH_H__ */
