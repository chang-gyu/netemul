CC			:= gcc
LD			:= ld 
SDK			:= ${HOME}/Project/netemul/sdk
INCLUDES	:= -I. -I$(TOPDIR)/include -I $(SDK) -I../librpc
DEFINES		:= -DNET_CONTROL -D__LINUX #-DDEBUG
LIBS		:= ../librpc/librpc_netemul.a ../libumpn.a
WARNING		:= -Wall -Werror -Wno-unused-result #-Wno-unused-function
CFLAGS		:= $(WARNING) -std=gnu99 -fms-extensions $(INCLUDES) $(DEFINES) $(LIBS)

.SUFFIXES : .o .c .S

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@
