# Check if required directories are defined
ifndef CF4OCLDIR
CF4OCLDIR := $(realpath ${CURDIR}/..)
$(warning CF4OCLDIR is not defined, assuming default $(CF4OCLDIR))
endif
ifndef OBJDIR
OBJDIR := $(CF4OCLDIR)/obj
$(warning OBJDIR is not defined, assuming default $(OBJDIR))
endif
ifndef BUILDDIR
BUILDDIR := $(CF4OCLDIR)/bin
$(warning BUILDDIR is not defined, assuming default $(BUILDDIR))
endif

# Variables definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 `pkg-config --cflags glib-2.0` -fopenmp 
CLMACROS = -DATI_OS_LINUX
LFLAGS = -lOpenCL -lm `pkg-config --libs glib-2.0`
UTILSINCLUDES = -I$(CF4OCLDIR)
UTILOBJS = $(OBJDIR)/clutils.o $(OBJDIR)/clprofiler.o $(OBJDIR)/clerrors.o
CLINCLUDES =
CLLIBDIR =

# If you use AMD APPSDK and Linux you may want to uncomment the line bellow
#CLMACROS += -DATI_OS_LINUX

# The location of the OpenCL headers
# In Ubuntu you can install the package opencl-headers, so that the line bellow remains commented
#CLINCLUDES += -I$$AMDAPPSDKROOT/include

# The location of libOpenCL.so
# If you have it in your LD_LIBRARY_PATH you can leave the line bellow commented
#CLLIBDIR = -L$$AMDAPPSDKROOT/lib/x86_64

# Phony targets
.PHONY: all clean mkdirs

# Make rules
all: mkdirs bank_conflicts matmult

bank_conflicts: bank_conflicts.o exp_common.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(UTILOBJS) $(LFLAGS)
	cp bank_conflicts.cl $(BUILDDIR)

bank_conflicts.o: bank_conflicts.c 
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) $(UTILSINCLUDES) -c $< -o $(OBJDIR)/$@

matmult.o: matmult.c matmult.h
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) $(UTILSINCLUDES) -c $< -o $(OBJDIR)/$@

matmult: matmult.o exp_common.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(UTILOBJS) $(LFLAGS)
	cp matmult.cl $(BUILDDIR)

exp_common.o: exp_common.c exp_common.h
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) $(UTILSINCLUDES) -c $< -o $(OBJDIR)/$@

mkdirs:
	mkdir -p $(BUILDDIR)
	mkdir -p $(OBJDIR)
	
clean:
	rm -rf $(OBJDIR) $(BUILDDIR)
