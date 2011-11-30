# -*- mode: Makefile;-*-
# Makefile -- random

include config.mk
-include local.mk

SRCROOT=.
INCLUDE_FLAGS=-I MBLib/public

SRCDIR=$(SRCROOT)
BUILDDIR=$(BUILDROOT)
DEPDIR=$(DEPROOT)

#The BUILDROOT folder is included for config.h
CFLAGS = ${DEFAULT_CFLAGS} -I $(BUILDROOT) $(INCLUDE_FLAGS)
CPPFLAGS = ${CFLAGS}

#LIBFLAGS = -lm

CC=gcc
CXX=g++

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	${CC} -c ${CFLAGS} -o $(BUILDDIR)/$*.o $<;

#Autogenerate dependencies information
#The generated makefiles get sourced into this file later
$(DEPDIR)/%.d: $(SRCDIR)/%.c Makefile
	$(CC) -M -MM -MT "$(BUILDDIR)/$*.o" -MT "$(DEPDIR)/$*.d" -MF $@ ${CFLAGS} $<;

#No paths. VPATH is assumed
SOURCES = main.c

#For reasons I cannot fathom, MBLIB_OBJ has to be last
# or things don't link right...

MBLIB_OBJ = $(MBLIB_BUILDDIR)/MBLib.a

RANDOM_OBJ = $(addprefix $(BUILDDIR)/, $(subst .c,.o, $(SOURCES))) $(MBLIB_OBJ)
		     
.PHONY: all clean distclean dist MBLib

#The config check is to test if we've been configured
all: config.mk $(BUILDROOT)/config.h random

$(MBLIB_OBJ): MBLib

MBLib:
	$(MAKE) -f $(MBLIB_SRCDIR)/Makefile all

random: $(BUILDROOT)/random

$(BUILDROOT)/random: $(RANDOM_OBJ)
	${CC} ${CFLAGS} ${LIBFLAGS} $(RANDOM_OBJ) -o $(BUILDROOT)/random

# XXX: I don't yet have a way to auto create the build dirs before
#      building... 
# clean leaves the dep files
clean:
	$(MAKE) -f $(MBLIB_SRCDIR)/Makefile clean
	rm -f $(TMPDIR)/*
	rm -f $(BUILDDIR)/*.o
	rm -f $(BUILDROOT)/random

# after a distclean you'll need to run configure again
distclean: clean
	rm -rf $(BUILDROOT)/
	rm -rf config.mk

#include the generated dependency files
-include $(addprefix $(DEPDIR)/,$(subst .c,.d,$(SOURCES)))



