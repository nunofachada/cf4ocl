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

# If you use AMD APPSDK and Linux you may want to uncomment the line bellow
#CLMACROS += -DATI_OS_LINUX
CLMACROS +=

# The location of the OpenCL headers
# In Debian/Ubuntu you can install the package opencl-headers, so that the line bellow remains commented
#CLINCLUDES += -I$$AMDAPPSDKROOT/include
CLINCLUDES +=

# The location of libOpenCL.so
# If you have it in your LD_LIBRARY_PATH you can leave the line bellow commented
# You can also install the ocl-icd-opencl-dev package in order to leave the line bellow commented
#CLLIBDIR += -L$$AMDAPPSDKROOT/lib/x86_64
CLLIBDIR +=

# Variables Used by Implicit Rules
CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99 `pkg-config --cflags glib-2.0` -fopenmp -I$(CF4OCL_INCDIR) $(CLINCLUDES) $(CLMACROS)
LDFLAGS = $(CLLIBDIR)
LDLIBS = -lOpenCL -lm `pkg-config --libs glib-2.0`

# Available utilities
UTILS := device_query kernel_info

# Binaries associated with the available utilities
BINS := $(addprefix $(BUILDSUBDIR)/,$(UTILS))

# Objects associated with the available utilities
OBJS := $(addprefix $(OBJDIR)/,$(foreach var,$(UTILS),$(var).o))

# Library objects
CF4OCL_OBJS := $(addprefix $(OBJDIR)/,clutils.o clprofiler.o clerrors.o)

# Phony rules
.PHONY : all clean $(UTILS)

# Make rules

all : $(UTILS)

$(UTILS) : % : $(BUILDSUBDIR)/%

$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BINS) : $(CF4OCL_OBJS)
$(BINS) : $(BUILDSUBDIR)/% : $(OBJDIR)/%.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

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
