TOP = ../../..
ifdef EPICS_HOST_ARCH
  include $(TOP)/configure/CONFIG
   
   
   USR_CXXFLAGS_Linux += -DSINGLE_THREADED=1 -D__epics__=1
   
   EdmChoiceButton_LIBS_Linux += EdmBase EdmLib
   EdmChoiceButton_SYS_LIBS_Linux += Xm Xt Xp Xtst X11 pthread dl
   
   
   USR_CXXFLAGS_solaris += -D__solaris__ -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_solaris = EdmBase EdmLib Xm Xt Xmu X11 Xext pthread dl rt

   USR_CXXFLAGS_hpux11_11_mt += -DHP_UX=1111 -DSINGLE_THREADED=1 -D__epics__=1
   USR_LIBS_hpux11_11_mt = EdmLib pthread
   
   
   
   USR_LDFLAGS += -L$(INSTALL_LIB)
   EdmChoiceButton_LDFLAGS += -L$(MOTIF_LIB) -L$(X11_LIB)
   
   LIBRARY_HOST = EdmChoiceButton

   PROD_LIBS += ca Com

   USR_INCLUDES = -I../ -I../../util -I../../lib -I../../pvlib
   USR_INCLUDES += -I$(TOP)/include/sys/linux
   USR_INCLUDES += -I$(TOP)/include/net/linux
   USR_INCLUDES += -I$(TOP)/include/msg
   USR_INCLUDES += -I$(TOP)/include/thread/linux
   USR_INCLUDES += -I$(TOP)/include/iprpc/osf
   USR_INCLUDES += -I$(TOP)/include/avl
   USR_INCLUDES += -I$(TOP)/include/crc
   USR_INCLUDES += -I$(MOTIF_INC) -I$(X11_INC)

   Xm_DIR =   $(MOTIF_LIB)
   Xt_DIR =   $(X11_LIB)
   X11_DIR =  $(X11_LIB)
   Xmu_DIR =  $(X11_LIB)
   Xtst_DIR = $(X11_LIB)
  
   # ELW added following two lines for
   # Red Hat 7.0 LINUX
   Xp_DIR =   $(X11_LIB)
   Xext_DIR = $(X11_LIB)
   #
   XlibGUI_DIR = $(X11_LIB)
   XmStatic_DIR = $(MOTIF_LIB)
   XmStatXt_DIR = $(MOTIF_LIB)
   HCLXt_DIR = $(X11_LIB)
   Xlib_DIR = $(X11_LIB)
   HCLXmu_DIR = $(X11_LIB)
   
  
   LIB_SRCS += choice_button.cc
   LIB_SRCS += reg_libEdmChoiceButton.cc
   
   

   include $(TOP)/configure/RULES
   
else
   ifneq ($(wildcard $(TOP)/config)x,x)
     # New Makefile.Host config file location
     include $(TOP)/config/CONFIG_EXTENSIONS
     include $(TOP)/config/RULES_ARCHS
   else
     # Old Makefile.Unix config file location
     EPICS=../../..
     include $(EPICS)/config/CONFIG_EXTENSIONS
     include $(EPICS)/config/RULES_ARCHS
   endif
endif

