
#if DEBUG
		printf("Broadcast packet : ");
		uint8_t mac[6];
		for(int i = 0; i < 6; i++) 
			mac[i] = (ether->dmac >> (5 - i) * 8) & 0xff;

		printf("\t%02x:%02x:%02x:%02x:%02x:%02x\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]); 
#endif


#if DEBUG
	int len = packet->end - packet->start;

	printf("\n%s send (len %d)\n", this->type == NODE_TYPE_ETHER_SWITCH ? "Switch": "Hub", len);
	for(int i = 0; i < len; i++) {
		if(i % 16 == 0)
			printf("\n%04x:\t", i);

		if(i % 2 == 0)
			printf(" ");

		printf("%02x", packet->buffer[i]);
	}
	printf("\n");

#endif

#if DEBUG
		printf("Unicast packet\n");
		uint8_t mac[6];
		for(int i = 0; i < 6; i++) 
			mac[i] = (ether->dmac >> (5 - i) * 8) & 0xff;

		printf("\t%02x:%02x:%02x:%02x:%02x:%02x\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]); 
#endif
