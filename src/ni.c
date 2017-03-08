#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>

#ifndef __LINUX
#include <net/nic.h>
#endif

#include "ni.h"
#include "manager.h"

#include <sys/types.h>
#include <net/if.h>
#include <net/ethernet.h>

#include <linux/if_packet.h>
//#include <net/if_arp.h>
#include <arpa/inet.h>


static int do_chflags(const char *dev, uint32_t flags, uint32_t mask) {
	int get_ctl_fd(void) {
		int s_errno;
		int fd;

		fd = socket(PF_INET, SOCK_DGRAM, 0);
		if(fd >= 0)
			return fd;
		s_errno = errno;
		fd = socket(PF_PACKET, SOCK_DGRAM, 0);
		if(fd >= 0)
			return fd;
		fd = socket(PF_INET6, SOCK_DGRAM, 0);
		if(fd >= 0)
			return fd;
		errno = s_errno;
		perror("Cannot create control socket");

		return -1;
	}

	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	fd = get_ctl_fd();
	if(fd < 0)
		return -1;

	if((err = ioctl(fd, SIOCGIFFLAGS, &ifr)) < 0) {
		perror("ioctl(SIOCGIFFLAGS)");
		close(fd);
		return -1;
	}

	if((ifr.ifr_flags ^ flags) & mask) {
		ifr.ifr_flags &= ~mask;
		ifr.ifr_flags |= mask & flags;

		if((err = ioctl(fd, SIOCSIFFLAGS, &ifr)) < 0) {
			perror("ioctl(SIOCSIFFLAGS)");
			close(fd);
			return -1;
		}
	}

	/*
	   if((err = ioctl(fd, SIOCGIFHWADDR, &ifr)) < 0) {
	   perror("ioctl(SIOCGIFHWADDR)");
	   close(fd);
	   return -1;
	   }

	   uint64_t* mac = (uint64_t*)ifr.ifr_hwaddr.sa_data;
	   ti->mac = endian48(*mac);
	   */

	close(fd); 

	return err;
}

static TapInterface* tap_create(const char* name, int flags) {
	struct ifreq ifr;
	int fd, err;
	char *dev = "/dev/net/tun";

	if((fd = open(dev, O_RDWR)) < 0) {
		perror("Cannot open TUN device"); 
		exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;   

	if(*dev) {
		strncpy(ifr.ifr_name, name, IFNAMSIZ);
	}

	if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
		perror("ioctl(TUNSETIFF)");
		close(fd);
		exit(1);
	}

	TapInterface* ti = (TapInterface*)malloc(sizeof(TapInterface));
	if(!ti)
		return NULL;

	ti->fd = fd;
	strcpy(ti->name, ifr.ifr_name);	

	// Set tap interface up
	if(do_chflags(ifr.ifr_name, IFF_UP, IFF_UP) < 0) {
		perror("Interface up failed\n");
		exit(1);	
	}

	return ti;
}

static void tap_destroy(TapInterface* ti) { 
	close(ti->fd);
	free(ti);
	ti = NULL;
}
#ifndef __LINUX
void ni_init() {
	Manager* manager = get_manager();
	manager->nic_count = 0;

	for(int i = 0; i < nic_count(); i++) {
		NI* ni = (NI*)malloc(sizeof(NI));
		ni->used = 0; // unused == 0.
		printf("ni->used : %d\n", ni->used);
		manager->nis[i] = ni; //(NI*)malloc(sizeof(NI));
	}
}

static NI* nic_create(const char* name, int index) {
	Manager* manager = get_manager();

	int i;
	for(i = 0; i < manager->nic_count; i++) {
		if(manager->nis[i]->used == 0) {
			manager->nis[i]->used = 1;
			//strncpy(manager->nis[i]->nic->name, name, strlen(name));
			break;
		}
	}

	return manager->nis[i];
}
#endif
/*
NI* ni_attach(EndPointPort* port) {
	Node* temp = (Node*)port->owner;
	char ifname[8];
	strcpy(ifname, temp->name);
	printf("input ifname : %s\n", ifname);			//debug;
	NI* ni = malloc(sizeof(NI));
	if(!ni)
		return NULL;

	TapInterface* ti = (TapInterface*)malloc(sizeof(TapInterface));
	if(!ti)
		return NULL;

*/
//read mac;
//
//#define IFRSIZE	((int)(size * sizesof(struct ifreq)))
//	unsigned char      *u; 
//	int                sockfd, size  = 1;
//	struct ifreq       *ifr;
//	struct ifconf      ifc;
//	struct sockaddr_in sa; 
//
//	if (0 > (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))) {
//		fprintf(stderr, "Cannot open socket.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	ifc.ifc_len = IFRSIZE;
//	ifc.ifc_req = NULL;
//
//	do {
//		++size;
//		/* realloc buffer size until no overflow occurs  */
//		if (NULL == (ifc.ifc_req = realloc(ifc.ifc_req, IFRSIZE))) {
//			fprintf(stderr, "Out of memory.\n");
//			exit(EXIT_FAILURE);
//		}
//		ifc.ifc_len = IFRSIZE;
//		if (ioctl(sockfd, SIOCGIFCONF, &ifc)) {
//			perror("ioctl SIOCFIFCONF");
//			exit(EXIT_FAILURE);
//		}
//	} while  (IFRSIZE <= ifc.ifc_len);
//
//	ifr = ifc.ifc_req;
//	for (;(char *) ifr < (char *) ifc.ifc_req + ifc.ifc_len; ++ifr) {
//
//		if (ifr->ifr_addr.sa_data == (ifr+1)->ifr_addr.sa_data) {
//			continue;  /* duplicate, skip it */
//		}   
//
//		if (ioctl(sockfd, SIOCGIFFLAGS, ifr)) {
//			continue;  /* failed to get flags, skip it */
//		}   
//
//		if(!strcmp(ifr->ifr_name, ifname)) {
//
//			if (0 == ioctl(sockfd, SIOCGIFHWADDR, ifr)) {
//
//				switch (ifr->ifr_hwaddr.sa_family) {
//					default:
//						continue;
//					case  ARPHRD_NETROM:  case  ARPHRD_ETHER:  case  ARPHRD_PPP:
//					case  ARPHRD_EETHER:  case  ARPHRD_IEEE802: break;
//				}   
//
//				u = (unsigned char *) &ifr->ifr_addr.sa_data;
//
//
//				if (u[0] + u[1] + u[2] + u[3] + u[4] + u[5]) {
//					printf("HW Address: %2.2x.%2.2x.%2.2x.%2.2x.%2.2x.%2.2x\n",
//							u[0], u[1], u[2], u[3], u[4], u[5]);
//				}   
//			}   
//		}
//	}   
//	close(sockfd);
/*
	struct sockaddr_ll daddr;

	int sock = socket( PF_PACKET , SOCK_RAW , htons(ETH_P_ALL)) ;            //For sending

	memset(&daddr, 0, sizeof(struct sockaddr_ll));
	daddr.sll_family = AF_PACKET;
	daddr.sll_protocol = htons(ETH_P_ALL);
	daddr.sll_ifindex = if_nametoindex(ifname);
	if (bind(sock, (struct sockaddr*) &daddr, sizeof(daddr)) < 0) {
	//	perror("bind failed\n");
		close(sock);
	}   

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", ifname);
	if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
	//	perror("bind to %s", ifname);
	}   
	ti->fd = sock;
	strcpy(ti->name, ifname);

	ni->ti = ti;
	ni->port = port;

	return ni;
}
	*/
NI* ni_create(EndPointPort* port) {
#ifdef __LINUX
	NI* ni = malloc(sizeof(NI));
	if(!ni)
		return NULL;

	TapInterface* ti = tap_create(port->name, IFF_TAP | IFF_NO_PI);
	if(!ti) 
		goto failed;

	ni->ti = ti;
	ni->port = port;

	if(!fd_add(ti->fd))
		goto failed;

	return ni;

failed:
	if(ti)
		tap_destroy(ti);

	if(ni) {
		free(ni);
		ni = NULL;
	}
	return NULL;
#else
	NI* ni = nic_create(port->name, 1);
	ni->port = port;

	return ni;
#endif
}

void ni_destroy(NI* ni) {
#ifdef __LINUX 
	fd_remove(ni->ti->fd);
	tap_destroy(ni->ti);
	free(ni);
	ni = NULL;
#else
	ni->used = 0;
	ni->nic = NULL;
#endif
}

