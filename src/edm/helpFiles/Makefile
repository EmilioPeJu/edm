TOP = ../../..
ifdef EPICS_HOST_ARCH
   include $(TOP)/configure/CONFIG

   HTMLS_DIR=edm
   HTMLS+=$(patsubst ../%, %, $(wildcard ../*.htm))
   HTMLS+=$(patsubst ../%, %, $(wildcard ../*.png))
   HTMLS+=$(patsubst ../%, %, $(wildcard ../*.gif))
   HTMLS+=$(patsubst ../%, %, $(wildcard ../*.edl))
   HTMLS+=style.css

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
