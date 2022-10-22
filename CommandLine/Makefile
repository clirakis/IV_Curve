##################################################################
#
#	Makefile for gpib code using gcc on Linux. 
#
#
#	Modified	by	Reason
# 	--------	--	------
#	19-Jan-14       CBL     Original
#
#
######################################################################
# Machine specific stuff
#
#
COMMON = $(HOME)/common
TARGET = Keith
#
# Compile time resolution.
#
EXT_CFLAGS +=  -std=gnu++11
INCLUDE =  -I$(COMMON)/GPIB -I$(COMMON)/Keithley -I$(COMMON)/utility \
	-I/usr/local/include -I$(ROOT_INC)
LIBS = -L$(DRIVE)/lib_linux -lKeithley -lmygpib -lutility \
	-L/usr/local/lib -lgpib $(ROOT_GLIBS)

# Rules to make the object files depend on the sources.
SRC     = 
SRCCPP  = main.cpp 
SRCS    = $(SRC) $(SRCCPP)

HEADERS = 
# When we build all, what do we build?
all:      $(TARGET)

#
# The dictionaries are generated automatically using the rootcint
# program. They are based on the headers and do some magic
# with respect to the ability of root to call my local classes.
# This is a Qt thing, so if you want to know more about it
# look at the Qt documentation.
#
#S_Dict.cpp: 	$(HEADERS)
#	@echo "Generating builder root dictionary $@..."
#	rootcint -f $@ -c $(HEADERS)

include $(DRIVE)/common/makefiles/makefile.inc


#dependencies
include make.depend 
# DO NOT DELETE
