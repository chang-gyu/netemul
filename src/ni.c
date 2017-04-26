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

NI* ni_create(VirtualPort* port) {
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
}

void ni_destroy(NI* ni) {
	fd_remove(ni->ti->fd);
	tap_destroy(ni->ti);
	free(ni);
	ni = NULL;
}

