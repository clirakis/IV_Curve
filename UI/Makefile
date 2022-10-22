##################################################################
#
#	Makefile for IVplot using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	22-Nov-18       CBL     Original
#
######################################################################
# Machine specific stuff
#
#
TARGET = IVplot
#
# Compile time resolution.
#
EXT_CFLAGS +=  -std=gnu++11 -g
INCLUDE = -I$(COMMON)/GPIB -I$(COMMON)/Keithley -I$(DRIVE)/common/utility \
	-I$(ROOT_INC)
LIBS = -L$(HOME)/lib_linux -lKeithley -lmygpib -lutility \
	-L/usr/local/lib -lgpib $(ROOT_GLIBS) 

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp IVcurve.cpp ParamDialog.cpp ParamPane.cpp IV_Dict.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = IVcurve.hh ParamDialog.hh ParamPane.hh IV_Linkdef.hh

# When we build all, what do we build?
all:      $(TARGET)


#
# The dictionaries are generated automatically using the rootcint
# program. They are based on the headers and do some magic
# with respect to the ability of root to call my local classes.
# This is a Qt thing, so if you want to know more about it
# look at the Qt documentation.
#
IV_Dict.cpp: 	$(HEADERS)
	@echo "Generating builder root dictionary $@..."
	rootcint -f $@ -c $(HEADERS)

include $(DRIVE)/common/makefiles/makefile.inc

#dependencies
#include make.depend 
