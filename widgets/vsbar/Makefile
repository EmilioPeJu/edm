TOP = ../../../..

include $(TOP)/configure/CONFIG

EDM=../../..
USR_INCLUDES=-I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib
USR_INCLUDES += -I$(TOP)/include/sys/linux
USR_INCLUDES += -I$(TOP)/include/thread/linux
USR_INCLUDES += -I$(TOP)/include/avl

LIBRARY_HOST = EdmVsbar
LIB_SRCS += vsbar.cc

include $(TOP)/configure/RULES
