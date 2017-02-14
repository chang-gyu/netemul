.EXPORT_ALL_VARIABLES:

TARGET	:= netemul
TOPDIR	:= $(shell /bin/pwd)
SUBDIRS	:= src

include $(TOPDIR)/Config.mk

all : compile $(OBJS)
	$(CC) $(OBJS) $(addsuffix /built-in.o, $(SUBDIRS)) -o $(TARGET)

include $(TOPDIR)/Rules.mk

