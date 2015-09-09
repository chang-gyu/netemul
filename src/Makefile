ifndef $(TOPDIR)
	TOPDIR := ../
endif
include $(TOPDIR)/Config.mk

.PHONY: version.h

TARGET	:= built-in.o
SUBDIRS := 
OBJS	:= $(patsubst %.c, %.o, $(wildcard *.c))

all : version.h compile $(OBJS)
	$(LD) -r $(addsuffix /$(TARGET), $(SUBDIRS)) $(OBJS) $(LIBS) -o $(TARGET)

version.h:
	rm -rf version.h
	./mkver.sh > version.h

include $(TOPDIR)/Rules.mk
