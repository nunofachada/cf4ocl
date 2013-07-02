# Check if required directories are defined
ifndef OBJDIR
OBJDIR := ${CURDIR}/obj
$(warning OBJDIR is not defined, assuming default $(OBJDIR))
endif
ifndef BUILDDIR
BUILDDIR := ${CURDIR}/bin
$(warning BUILDDIR is not defined, assuming default $(BUILDDIR))
endif
ifndef UTILSINCLUDEDIR
UTILSINCLUDEDIR := $(realpath ${CURDIR}/..)
$(warning UTILSINCLUDEDIR is not defined, assuming default $(UTILSINCLUDEDIR))
endif

# Variables definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 `pkg-config --cflags glib-2.0` -fopenmp 
CLMACROS = -DATI_OS_LINUX
LFLAGS = -lOpenCL -lm `pkg-config --libs glib-2.0`
UTILSINCLUDES = -I$(UTILSINCLUDEDIR)
UTILOBJS = $(OBJDIR)/bitstuff.o $(OBJDIR)/clutils.o $(OBJDIR)/clprofiler.o
CLINCLUDES =
CLLIBDIR =

# If you use AMD APPSDK you may want to uncomment the following lines
#CLINCLUDES += -I$$AMDAPPSDKROOT/include
#CLLIBDIR += -L$$AMDAPPSDKROOT/lib/x86_64

# Phony targets
.PHONY: all clean mkdirs

# Make rules
all: mkdirs BankConflictTest matmult

BankConflictTest: BankConflictTest.o exp_common.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(UTILOBJS) $(LFLAGS)
	cp BankConflictTest.cl $(BUILDDIR)

BankConflictTest.o: BankConflictTest.c 
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
