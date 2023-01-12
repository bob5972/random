# -*- mode: Makefile;-*-
#
# Makefile -- random

include config.mk
-include local.mk

SRCROOT=.
INCLUDE_FLAGS=-I MBLib/public

SRCDIR=$(SRCROOT)
BUILDDIR=$(BUILDTYPE_ROOT)
DEPDIR=$(DEPROOT)

#Final binary
TARGET=random

#The BUILDROOT folder is included for config.h
CFLAGS = ${DEFAULT_CFLAGS} -I $(BUILDDIR) $(INCLUDE_FLAGS)
CPPFLAGS = ${CFLAGS}

#LIBFLAGS = -lm

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	${CC} -c ${CFLAGS} -o $(BUILDDIR)/$*.o $<;
$(BUILDDIR)/%.opp: $(SRCDIR)/%.cpp
	${CXX} -c ${CPPFLAGS} -o $(BUILDDIR)/$*.opp $<;

#Autogenerate dependencies information
#The generated makefiles get sourced into this file later
ifeq ($(CC), gcc)
$(DEPDIR)/%.d: $(SRCDIR)/%.c Makefile
	$(CC) -M -MM -MT "$(BUILDDIR)/$*.o" -MT "$(DEPDIR)/$*.d" -MF $@ ${CFLAGS} $<;
$(DEPROOT)/%.dpp: $(SRCDIR)/%.cpp Makefile
	$(CXX) -M -MM -MT "$(BUILDDIR)/$*.opp" \
	    -MT "$(DEPDIR)/$*.dpp" \
	    -MF $@ ${CPPFLAGS} $<;
endif

#No paths. VPATH is assumed
C_SOURCES =
CPP_SOURCES = main.cpp

#For reasons I cannot fathom, MBLIB_OBJ has to be last
# or things don't link right...

MBLIB_OBJ = $(MBLIB_BUILDDIR)/MBLib.a

C_OBJ=$(addprefix $(BUILDDIR)/, $(subst .c,.o, $(C_SOURCES)))
CPP_OBJ=$(addprefix $(BUILDDIR)/, $(subst .cpp,.opp, $(CPP_SOURCES)))
TARGET_OBJ = $(C_OBJ) $(CPP_OBJ) $(MBLIB_OBJ)

.PHONY: all clean distclean dist $(TARGET)

#The config check is to test if we've been configured
all: config.mk $(BUILDDIR)/config.h $(TARGET)

# Our dependencies here are overly broad, which sometimes means that
# we'll make MBLib thinking something has changed, but the underlying
# MBLib build won't do anything, and we'll always think it needs
# updating for every subsequent build.
#
# It's not a big deal, but we can avoid it by touching the archive
# here after we build MBLib, so that it'll give it a newer timestamp
# than whatever false depenency was triggering the rebuild.
$(MBLIB_OBJ): MBLib/* MBLib/public/*
	$(MAKE) -f $(MBLIB_SRCDIR)/Makefile all
	touch $(MBLIB_OBJ)

$(TARGET): $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(TARGET_OBJ)
	${CXX} ${CFLAGS} $(TARGET_OBJ) $(LIBFLAGS) -o $(BUILDDIR)/$(TARGET)

clean:
	rm -rf $(BUILDROOT)/debug
	rm -rf $(BUILDROOT)/develperf
	rm -rf $(BUILDROOT)/release
	rm  -f $(BUILDROOT)/$(TARGET)

# after a distclean you'll need to run configure again
dist-clean: distclean
distclean: clean
	rm -rf $(BUILDROOT)/
	rm -rf config.mk

install: random
	cp -i build/random ~/bin/local/random

#include the generated dependency files
ifeq ($(CC), gcc)
-include $(addprefix $(DEPDIR)/,$(subst .c,.d,$(C_SOURCES)))
-include $(addprefix $(DEPDIR)/,$(subst .cpp,.dpp,$(CPP_SOURCES)))
endif
