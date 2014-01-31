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

# Need to export this for lib compilation
export OBJDIR

# Variables definitions
CC = gcc
CFLAGS = -Wall -Wextra -g -std=gnu99 `pkg-config --cflags glib-2.0` -fopenmp
LFLAGS = -lOpenCL -lm `pkg-config --libs glib-2.0`
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

EXAMPLES := bank_conflicts matmult
BINS := $(addprefix $(BUILDDIR)/,$(EXAMPLES))
OBJS := $(addprefix $(OBJDIR)/,$(foreach var,$(EXAMPLES),$(var).o)) $(OBJDIR)/exp_common.o
CF4OCL_OBJS := $(addprefix $(OBJDIR)/,clutils.o clprofiler.o clerrors.o)

# Phony targets
.PHONY : all clean $(EXAMPLES)

# Make rules
all : $(EXAMPLES)

$(EXAMPLES) : % : $(BUILDDIR)/%

$(OBJDIR)/%.o : %.c %.h
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) $(CF4OCL_INC) -c $< -o $@

$(BINS) : $(CF4OCL_OBJS)
$(BINS) : $(BUILDDIR)/% : $(OBJDIR)/%.o $(OBJDIR)/exp_common.o | $(BUILDDIR)/%.cl
	$(CC) $(CFLAGS) $(CLMACROS) $^ $(CLLIBDIR) -o $@ $(LFLAGS)


$(BUILDDIR)/%.cl : %.cl | $(BUILDDIR)
	cp $< $@

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
