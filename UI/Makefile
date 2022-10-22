##################################################################
#
#	Makefile for IVplot using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	22-Nov-18       CBL     Original
#       22-Oct-22       CBL     Reconfigured, instruments external.
#
######################################################################
# Machine specific stuff
#
#
TARGET = IVplot
#
# Compile time resolution.
# Turn on debug
# Use cern root as well. 
# Libraries are Keithly, GPIB, and utility from me. 
#
EXT_CFLAGS +=  -g
INCLUDE = -I$(COMMON)/GPIB -I$(COMMON)/Keithley -I$(DRIVE)/common/utility \
	-I$(ROOT_INC)

LIBS = -L$(HOME)/lib_linux -lKeithley -lmygpib -lutility \
	-L/usr/local/lib -lgpib $(ROOT_GLIBS) 

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp IVcurve.cpp Instruments.cpp ParamDialog.cpp \
	ParamPane.cpp IV_Dict.cpp
SRCS    = $(SRC) $(SRCCPP)

HEADERS = IVcurve.hh Instruments.hh ParamDialog.hh ParamPane.hh IV_Linkdef.hh

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
