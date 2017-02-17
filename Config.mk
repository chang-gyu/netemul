CC			:= gcc
LD			:= ld 
SDK			:= $(HOME)/rtos/
INCLUDES	:= -I. -I$(TOPDIR)/include -I $(SDK)/include 
DEFINES		:= -DNET_CONTROL -D__LINUX #-DDEBUG
LIBS		:= $(SDK)/lib/libumpn.a
WARNING		:= -Wall -Werror -Wno-unused-result -Wno-unused-function
CFLAGS		:= $(WARNING) -std=gnu99 -fms-extensions $(INCLUDES) $(DEFINES) $(LIBS)

.SUFFIXES : .o .c .S

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@
