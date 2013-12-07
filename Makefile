# Check if required directories are defined by a higher priority Makefile
ifndef OBJDIR
export OBJDIR := ${CURDIR}/obj
endif
ifndef BASEBUILDDIR
BASEBUILDDIR := ${CURDIR}/bin
endif
ifndef CF4OCL_INCDIR
export CF4OCL_INCDIR := $(abspath ${CURDIR}/lib)
endif
ifndef CF4OCL_OBJDIR
export CF4OCL_OBJDIR := $(OBJDIR)
endif

export BUILDDIR = $(BASEBUILDDIR)/$@

# Phony rules
.PHONY : all lib utils examples tests docs clean cleanobj cleanbin cleandocs


# Make rules
all : lib utils examples

tests : cleanobj
utils examples : lib
lib utils examples tests :
	$(MAKE) -C $@

docs :
	sed -e 's/```c/~~~~~~~~~~~~~~~{.c}/g' \
	    -e 's/```/~~~~~~~~~~~~~~~/g'      \
	    README.md > README.doxy.md
	doxygen 
	cp -r images doc/html 
	rm README.doxy.md
	
clean : cleanobj cleanbin cleandocs
	
cleanobj :
	rm -rf $(OBJDIR)

cleanbin :
	rm -rf $(BASEBUILDDIR)

cleandocs :
	rm -rf doc

