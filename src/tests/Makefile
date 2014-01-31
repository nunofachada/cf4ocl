# Check if required directories are defined
ifndef CF4OCL_INCDIR
CF4OCL_INCDIR := $(abspath ${CURDIR}/../lib)
$(warning CF4OCL_INCDIR is not defined, assuming default $(CF4OCL_INCDIR))
endif
ifndef OBJDIR
OBJDIR := $(abspath ${CURDIR}/obj)
$(warning OBJDIR is not defined, assuming default $(OBJDIR))
endif
ifndef BUILDDIR
BUILDDIR := $(abspath ${CURDIR}/bin)
$(warning BUILDDIR is not defined, assuming default $(BUILDDIR))
endif

# Need to export these for lib compilation
export TESTMACROS = -DTESTING -I$(abspath ${CURDIR})
export OBJDIR

# Variables definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99 `pkg-config --cflags glib-2.0`
LFLAGS = -lOpenCL `pkg-config --libs glib-2.0`
CF4OCL_INC = -I$(CF4OCL_INCDIR)
CLMACROS +=
CLINCLUDES +=
CLLIBDIR +=

# If you use AMD APPSDK and Linux you may want to uncomment the line bellow
#CLMACROS += -DATI_OS_LINUX

# The location of the OpenCL headers
# In Ubuntu you can install the package opencl-headers, so that the line bellow remains commented
#CLINCLUDES += -I$$AMDAPPSDKROOT/include

# The location of libOpenCL.so
# If you have it in your LD_LIBRARY_PATH you can leave the line bellow commented
#CLLIBDIR = -L$$AMDAPPSDKROOT/lib/x86_64

TESTS := test_profiler test_gerrorf
BINS := $(addprefix $(BUILDDIR)/,$(TESTS))
OBJS := $(addprefix $(OBJDIR)/,$(foreach var,$(TESTS),$(var).o))
CF4OCL_OBJS := $(addprefix $(OBJDIR)/,clutils.o clprofiler.o clerrors.o)

# Phony rules
.PHONY : all clean $(TESTS)

# Make rules
all : $(TESTS)

$(TESTS) : % : $(BUILDDIR)/%

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) $(TESTMACROS) $(CF4OCL_INC) -c $< -o $@

$(BINS) : $(BUILDDIR)/% : $(OBJDIR)/%.o
	$(CC) $(CFLAGS) $(CLMACROS) $^ $(CLLIBDIR) -o $@ $(LFLAGS)

$(OBJDIR)/test_gerrorf.o : $(CF4OCL_INCDIR)/gerrorf.h
$(OBJDIR)/test_profiler.o : test_profiler.h
$(BUILDDIR)/test_profiler : $(CF4OCL_OBJS)

$(OBJS) : | $(OBJDIR)

$(BINS) : | $(BUILDDIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)
	
$(BUILDDIR) :
	mkdir -p $(BUILDDIR)

$(CF4OCL_OBJS):
	$(MAKE) -C $(CF4OCL_INCDIR)

clean :
	rm -rf $(OBJDIR) $(BUILDDIR)
