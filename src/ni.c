#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <arpa/inet.h>

#ifndef __LINUX
#include <net/nic.h>
#endif

#include "ni.h"
#include "manager.h"

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
	ni_context->mac = endian48(*mac);
	*/

	close(fd);

	return err;
}

static NI_Context* raw_create(const char* name, int flags) {
#define ETHER_TYPE 0x0800
    int fd;
    if((fd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == 1) {
        printf("Socket Error\n");
        return NULL;
    }

    NI_Context* ni_context = (NI_Context*)malloc(sizeof(NI_Context));
    if(!ni_context)
        return NULL;
    ni_context->fd = fd;

    struct ifreq ifr;
    //TODO: comment 
   // char ifName[IFNAMSIZ];
    char* ifName = name;
    strncpy(ifr.ifr_name, ifName, IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFFLAGS, &ifr);
//    ifr.ifr_flags |= flags;
    ifr.ifr_flags |= flags;
    ioctl(fd, SIOCSIFFLAGS, &ifr);

    int sockopt;

    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        printf("setsockopt error\n");
        return NULL;
    }

    if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ -1) == -1) {
        printf("so_bindtodevice\n");
        return NULL;
    }

    strcpy(ni_context->name, ifr.ifr_name);
    if(do_chflags(ifr.ifr_name, IFF_UP, IFF_UP) < 0) {
        perror("Interface up failed\n");
        exit(1);
    }

    return ni_context;
}

static NI_Context* tap_create(const char* name, int flags) {
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

	NI_Context* ni_context = (NI_Context*)malloc(sizeof(NI_Context));
	if(!ni_context)
		return NULL;

	ni_context->fd = fd;
	strcpy(ni_context->name, ifr.ifr_name);

	// Set tap interface up
	if(do_chflags(ifr.ifr_name, IFF_UP, IFF_UP) < 0) {
		perror("Interface up failed\n");
		exit(1);
	}

	return ni_context;
}

//tap___!!//
static void tap_destroy(NI_Context* ni_context) {
	close(ni_context->fd);
	free(ni_context);
	ni_context = NULL;
}

//FIXME: Port* port, type
NI* ni_create(VirtualPort* vport, PhysicalPort* pport) {
    NI* ni = malloc(sizeof(NI));
    NI_Context* ni_context;
    if(!ni)
        return NULL;

    if(vport) {
        ni_context = tap_create(vport->name, IFF_TAP | IFF_NO_PI);
        if(!ni_context)
            goto failed;

        ni->ni_context = ni_context;
        ni->vport = vport;
        ni->pport = NULL;

        if(!fd_add(ni_context->fd))
            goto failed;

        return ni;
    } else if(pport) {
        ni_context = raw_create(pport->ifname, IFF_PROMISC);
        if(!ni_context)
            goto failed;

        ni->ni_context = ni_context;
        ni->vport = NULL;
        ni->pport = pport;

        if(!fd_add(ni_context->fd))
            goto failed;

        return ni;
    } else
        goto failed;

failed:
    if(ni_context)
        tap_destroy(ni_context);

    if(ni) {
        free(ni);
        ni = NULL;
    }
    return NULL;
}

void ni_destroy(NI* ni) {
    fd_remove(ni->ni_context->fd);
    tap_destroy(ni->ni_context);
    free(ni);
    ni = NULL;
}

