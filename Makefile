# Check if required directories are defined by a higher priority Makefile
ifndef OBJDIR
export OBJDIR := ${CURDIR}/obj
endif
ifndef BUILDDIR
export BUILDDIR := ${CURDIR}/bin
endif

# Variables definitions
CC = gcc
CFLAGS = -Wextra -Wall -g -std=c99 `pkg-config --cflags glib-2.0`
CLLIB = -lOpenCL
LFLAGS  = `pkg-config --libs glib-2.0`
TESTSDIR = tests
UTILS_IN_BIN_DIR = utils
TESTS_IN_BIN_DIR = tests

TESTMACROS =
CLMACROS =
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

# Export utils include directory
export UTILSINCLUDEDIR := ${CURDIR}

# Phony rules
.PHONY: all tests tests_common examples clean mkdirs

# Make rules (examples and tests are not built by default)
all: mkdirs device_query kernel_info clutils.o clprofiler.o

device_query: device_query.o clutils.o
	$(CC) $(CFLAGS) $(CLMACROS) $(patsubst %,$(OBJDIR)/%,$^) $(CLINCLUDES) $(CLLIB) $(CLLIBDIR) -o $(BUILDDIR)/$(UTILS_IN_BIN_DIR)/$@ $(LFLAGS)
	
device_query.o: device_query.c
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) -c $< -o $(OBJDIR)/$@
	
kernel_info: kernel_info.o clutils.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$(UTILS_IN_BIN_DIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(LFLAGS) $(CLLIB) 
	
kernel_info.o: kernel_info.c
	$(CC) $(CFLAGS) $(CLMACROS) -c $< $(CLINCLUDES) -o $(OBJDIR)/$@
	
clutils.o: clutils.c clutils.h
	$(CC) $(CFLAGS) $(CLMACROS) $(CLINCLUDES) -c $< -o $(OBJDIR)/$@	

clprofiler.o: clprofiler.c clprofiler.h
	$(CC) $(CFLAGS) $(CLMACROS) $(TESTMACROS) $(CLINCLUDES) -c $< -o $(OBJDIR)/$@ 
	
tests: test_profiler test_gerrorf

test_profiler.o test_utils.o: mkdirs tests_common 

tests_common: 
	mkdir -p $(BUILDDIR)/$(TESTS_IN_BIN_DIR)

test_profiler: 	TESTMACROS += -DCLPROF_TEST
test_profiler: test_profiler.o clprofiler.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$(TESTS_IN_BIN_DIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(LFLAGS) $(CLLIB)
	
test_profiler.o: $(TESTSDIR)/test_profiler.c
	$(CC) $(CFLAGS) $(CLMACROS) $(TESTMACROS) -c $< $(CLINCLUDES) -o $(OBJDIR)/$@

test_gerrorf: test_gerrorf.o
	$(CC) $(CFLAGS) $(CLMACROS) $(CLLIBDIR) -o $(BUILDDIR)/$(TESTS_IN_BIN_DIR)/$@ $(patsubst %,$(OBJDIR)/%,$^) $(LFLAGS) $(CLLIB)
	
test_gerrorf.o: $(TESTSDIR)/test_gerrorf.c gerrorf.h
	$(CC) $(CFLAGS) $(CFLAGS_GLIB) $(CLMACROS) -c $< $(CLINCLUDES) -o $(OBJDIR)/$@
	
examples: mkdirs bitstuff.o clutils.o clprofiler.o
	$(MAKE) -C $@

mkdirs:
	mkdir -p $(BUILDDIR)
	mkdir -p $(BUILDDIR)/$(UTILS_IN_BIN_DIR)
	mkdir -p $(OBJDIR)
	
clean:
	rm -rf $(OBJDIR) $(BUILDDIR)

