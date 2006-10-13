TOP = ../../..
ifdef EPICS_HOST_ARCH
include $(TOP)/configure/CONFIG

USR_CFLAGS += -D__osf__=1

ifeq ($(EPICS_HOST_ARCH),$(T_A))
INSTALL_INCLUDE = $(TOP)/include/edm

SRC_DIRS += ../sys/$(OS_CLASS)
INC += os.h
INC += os_objs.h
INC += sys_types.h
INC += os_priv.h


SRC_DIRS += ../net/$(OS_CLASS)
INC += ipncl.h
INC += ipncl_priv.h
INC += ipnsv.h
INC += ipnsv_priv.h
INC += ipport.h
INC += ipport_priv.h
INC += ipnis.h
INC += ipnis_priv.h

SRC_DIRS += ../msg
INC += msg.h
INC += msg_priv.h


SRC_DIRS += ../thread/$(OS_CLASS)
INC += thread.h
INC += thread_priv.h


SRC_DIRS += ../iprpc/osf
INC += client_iprpc.h
INC += iprpc.h
INC += iprpc_priv.h
INC += server_iprpc_priv.h
INC += client_iprpc_priv.h
INC += server_iprpc.h

SRC_DIRS += ../avl
INC += avl.h
INC += avl_priv.h

SRC_DIRS += ../crc
INC += crc.h
endif

LIBRARY_HOST += EdmUtil
LIB_LIB += EdmUtil

LIB_SRCS += ipncl_pkg.c
LIB_SRCS += ipnsv_pkg.c
LIB_SRCS += ipnis_pkg.c

LIB_SRCS += sys_pkg.c
LIB_SRCS += os_pkg.c

LIB_SRCS += msg_pkg.c

LIB_SRCS += thread_pkg.c

LIB_SRCS += server_iprpc_pkg.c
LIB_SRCS += client_iprpc_pkg.c

LIB_SRCS += avl.c
LIB_SRCS += crc.c

include $(TOP)/configure/RULES


else
   ifneq ($(wildcard $(TOP)/config)x,x)
     # New Makefile.Host config file location
     include $(TOP)/config/CONFIG_EXTENSIONS
     include $(TOP)/config/RULES_ARCHS
   else
     # Old Makefile.Unix config file location
     EPICS=../../../..
     include $(EPICS)/config/CONFIG_EXTENSIONS
     include $(EPICS)/config/RULES_ARCHS
   endif
   
endif

