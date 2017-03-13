CC			:= gcc
LD			:= ld 
SDK			:= $(HOME)/Project/rtos/
INCLUDES	:= -I. -I$(TOPDIR)/include -I $(SDK)/include -I../librpc
DEFINES		:= -DNET_CONTROL -D__LINUX #-DDEBUG
LIBS		:= ../libumpn.a ../librpc/librpc_netemul.a
WARNING		:= -Wall -Werror -Wno-unused-result #-Wno-unused-function
CFLAGS		:= $(WARNING) -std=gnu99 -fms-extensions $(INCLUDES) $(DEFINES) $(LIBS)

.SUFFIXES : .o .c .S

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@
