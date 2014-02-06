# Check if required directories are defined by a higher priority Makefile
ifndef OBJDIR
OBJDIR := ${CURDIR}/obj
endif

# If you use AMD APPSDK and Linux you may want to uncomment the line bellow
#CLMACROS += -DATI_OS_LINUX
CLMACROS +=

# The location of the OpenCL headers
# In Debian/Ubuntu you can install the package opencl-headers, so that the line bellow remains commented
#CLINCLUDES += -I$$AMDAPPSDKROOT/include
CLINCLUDES +=

# Variables Used by Implicit Rules
CC = gcc
CFLAGS = -Wextra -Wall -g -std=gnu99 `pkg-config --cflags glib-2.0` $(CLINCLUDES) $(CLMACROS) $(TESTMACROS)

# Available libraries
LIBS := clutils clprofiler clerrors

# Objects associated with the available libraries
OBJS := $(addprefix $(OBJDIR)/,$(foreach var,$(LIBS),$(var).o))

# Phony targets
.PHONY : all clean $(LIBS)

# Make rules

all : $(LIBS)

$(LIBS) : % : $(OBJDIR)/%.o

$(OBJDIR)/%.o : %.c %.h gerrorf.h
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/clutils.o $(OBJDIR)/clprofiler.o : clerrors.c clerrors.h

$(OBJS) : | $(OBJDIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

clean :
	rm -rf $(OBJDIR)


