# Check if required directories are defined by a higher priority Makefile
ifndef OBJDIR
export OBJDIR := ${CURDIR}/obj
endif
ifndef BASEBUILDDIR
export BASEBUILDDIR := ${CURDIR}/bin
endif
ifndef CF4OCL_INCDIR
export CF4OCL_INCDIR := $(abspath ${CURDIR}/src/lib)
endif
ifndef CF4OCL_OBJDIR
export CF4OCL_OBJDIR := $(OBJDIR)
endif

# Phony rules
.PHONY : all lib utils examples tests docs clean clean-all cleandocs


# Make rules
all : lib utils examples

lib utils examples tests :
	$(MAKE) -C src $@

docs :
	sed -e 's/```c/~~~~~~~~~~~~~~~{.c}/g' \
	    -e 's/```/~~~~~~~~~~~~~~~/g'      \
	    README.md > README.doxy.md
	doxygen 
	cp -r images doc/html 
	rm README.doxy.md
	
clean :
	rm -rf $(OBJDIR)
	rm -rf $(BASEBUILDDIR)
	
clean-all : clean cleandocs

cleandocs :
	rm -rf doc

