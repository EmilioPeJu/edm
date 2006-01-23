TOP = ../../../..

include $(TOP)/configure/CONFIG

EDM=../../..
USR_INCLUDES=-I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib
USR_INCLUDES += -I$(TOP)/include/sys/linux
USR_INCLUDES += -I$(TOP)/include/thread/linux
USR_INCLUDES += -I$(TOP)/include/avl
USR_INCLUDES += -I$(TOP)/include/crc

LIBRARY_HOST = menu_muxPV
LIB_SRCS += menu_muxPV.cc

include $(TOP)/configure/RULES
