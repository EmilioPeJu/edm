# Makefile for medm top level
TOP = ../../..
ifdef EPICS_HOST_ARCH
 include $(TOP)/configure/CONFIG
 DIRS = FE-widgets videowidget vsbar wheelswitch menu_muxPV logMeter
 include $(TOP)/configure/RULES_DIRS

else
    TOP = ../..
    ifneq ($(wildcard $(TOP)/config)x,x)
      # New Makefile.Host config file location
      include $(TOP)/config/CONFIG_EXTENSIONS
      DIRS = FE-widgets videowidget vsbar wheelswitch menu_muxPV logMeter
      include $(TOP)/config/RULES_DIRS
    else
      # Old Makefile.Unix config file location
      EPICS=../../..
      include $(EPICS)/config/CONFIG_EXTENSIONS
      DIRS = FE-widgets videowidget vsbar wheelswitch menu_muxPV logMeter
      include $(EPICS)/config/RULES_DIRS
    endif
endif
