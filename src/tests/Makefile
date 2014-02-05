# Check if required directories are defined
ifndef CF4OCL_INCDIR
CF4OCL_INCDIR := $(abspath ${CURDIR}/../lib)
$(warning CF4OCL_INCDIR is not defined, assuming default $(CF4OCL_INCDIR))
endif
ifndef OBJDIR
export OBJDIR := $(abspath ${CURDIR}/obj)
$(warning OBJDIR is not defined, assuming default $(OBJDIR))
endif
ifndef BUILDSUBDIR
BUILDSUBDIR := $(abspath ${CURDIR}/bin)
$(warning BUILDSUBDIR is not defined, assuming default $(BUILDSUBDIR))
endif

# Need to export this for lib compilation
export TESTMACROS = -DTESTING -I$(abspath ${CURDIR})

# If you use AMD APPSDK and Linux you may want to uncomment the line bellow
#CLMACROS += -DATI_OS_LINUX
CLMACROS +=

# The location of the OpenCL headers
# In Ubuntu you can install the package opencl-headers, so that the line bellow remains commented
#CLINCLUDES += -I$$AMDAPPSDKROOT/include
CLINCLUDES +=

# The location of libOpenCL.so
# If you have it in your LD_LIBRARY_PATH you can leave the line bellow commented
#CLLIBDIR = -L$$AMDAPPSDKROOT/lib/x86_64
CLLIBDIR +=

# Variables Used by Implicit Rules
CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99 `pkg-config --cflags glib-2.0` $(CLINCLUDES) -I$(CF4OCL_INCDIR) $(TESTMACROS) $(CLMACROS)
LDFLAGS = $(CLLIBDIR)
LDLIBS = -lOpenCL `pkg-config --libs glib-2.0`

# Available tests
TESTS := test_profiler test_gerrorf

# Binaries associated with the available tests
BINS := $(addprefix $(BUILDSUBDIR)/,$(TESTS))

# Objects associated with the available tests
OBJS := $(addprefix $(OBJDIR)/,$(foreach var,$(TESTS),$(var).o))

# Library objects
CF4OCL_OBJS := $(addprefix $(OBJDIR)/,clutils.o clprofiler.o clerrors.o)

# Phony rules
.PHONY : all clean $(TESTS)

# Make rules

all : $(TESTS)

$(TESTS) : % : $(BUILDSUBDIR)/%

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BINS) : $(BUILDSUBDIR)/% : $(OBJDIR)/%.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/test_gerrorf.o : $(CF4OCL_INCDIR)/gerrorf.h
$(OBJDIR)/test_profiler.o : test_profiler.h
$(BUILDSUBDIR)/test_profiler : $(CF4OCL_OBJS)

$(OBJS) : | $(OBJDIR)

$(BINS) : | $(BUILDSUBDIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)
	
$(BUILDSUBDIR) :
	mkdir -p $(BUILDSUBDIR)

$(CF4OCL_OBJS):
	$(MAKE) -C $(CF4OCL_INCDIR)

clean :
	rm -rf $(OBJDIR) $(BUILDSUBDIR)
