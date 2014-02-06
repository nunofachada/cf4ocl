# Check if required directories are defined by a higher priority Makefile
ifndef OBJDIR
export OBJDIR := ${CURDIR}/obj
endif
ifndef BUILDDIR
BUILDDIR := ${CURDIR}/bin
endif
ifndef CF4OCL_INCDIR
export CF4OCL_INCDIR := $(abspath ${CURDIR}/lib)
endif
ifndef CF4OCL_OBJDIR
export CF4OCL_OBJDIR := $(OBJDIR)
endif

export BUILDSUBDIR = $(BUILDDIR)/$@

# Phony rules
.PHONY : all lib utils examples tests clean cleanobj cleanbin

# Make rules
all : lib utils examples

tests : cleanobj
utils examples : lib
lib utils examples tests :
	$(MAKE) -C $@

clean : cleanobj cleanbin
	
cleanobj :
	rm -rf $(OBJDIR)

cleanbin :
	rm -rf $(BUILDDIR)


