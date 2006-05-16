TOP = ../../..
ifdef EPICS_HOST_ARCH
   include $(TOP)/configure/CONFIG

   TEMPLATES_DIR=edm
   TEMPLATES+=edmObjects edmPrintDef edmPvObjects 
   TEMPLATES+=colors.list fonts.list calc.list
   SCRIPTS=edm
   TARGETS=edm

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

% : ../%.sh
	msi -MEDM_CONFIGS=$(shell $(MKDIR) -p $(INSTALL_TEMPLATES)/edm; cd $(INSTALL_TEMPLATES)/edm; pwd) \
	    -MEDM_HELP=$(shell cd $(INSTALL_HTML)/edm; pwd)        \
	    -MEDM_INSTALL=$(shell cd $(INSTALL_BIN); pwd)           \
            -MEDM_LIBS=$(shell cd $(INSTALL_LIB); pwd)    -o$@ $<
