TOP = ../../../..

include $(TOP)/configure/CONFIG

EDM=../../..
USR_INCLUDES=-I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib
USR_INCLUDES += -I$(TOP)/include/sys/linux
USR_INCLUDES += -I$(TOP)/include/thread/linux
USR_INCLUDES += -I$(TOP)/include/avl
USR_INCLUDES += -I$(TOP)/include/crc

USR_LDFLAGS += -L/usr/X11R6/lib -lXpm

LIBRARY_HOST = TwoDProfileMonitor
LIB_SRCS += TwoDProfileMonitor.cc
LIB_SRCS += widget.cc

include $(TOP)/configure/RULES
