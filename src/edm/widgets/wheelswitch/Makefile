TOP = ../../../..

include $(TOP)/configure/CONFIG

EDM=../../..
USR_INCLUDES=-I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib
USR_INCLUDES += -I$(TOP)/include/edm

LIBRARY_HOST = EdmWheelswitch
LIB_SRCS += onekeypad.cc wheelSwitch.cc

include $(TOP)/configure/RULES
