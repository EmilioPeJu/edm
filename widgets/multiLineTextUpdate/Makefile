TOP = ../../../..

include $(TOP)/configure/CONFIG

EDM=../../..
USR_INCLUDES=-I$(EDM)/lib -I$(EDM)/util -I$(EDM)/pvlib
USR_INCLUDES += -I$(TOP)/include/edm

LIBRARY_HOST = multiLineTextUpdate
LIB_SRCS += multiLineTextUpdate.cc
LIB_SRCS += pv_register.cc

include $(TOP)/configure/RULES
