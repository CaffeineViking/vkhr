name = "vkhr"
config = "debug"

all: FORCE
	premake5 gmake
	make -j8 -C build config=${config}
run: FORCE
	premake5 gmake
	make -j8 -C build ${name} config=${config}
	bin/${name} ${args}

docs: FORCE
	make -C docs
	doxygen docs/Doxyfile
	mv docs/html docs/reference
	make -C docs/latex
	mv docs/latex/refman.pdf docs/manual.pdf
tags: FORCE
	ctags -R src/${name} include/${name}
	cscope -Rb -s src/${name} -s include/${name}

clean: FORCE
	rm -rf build
	rm -rf docs/latex
	rm -rf docs/build
distclean: clean
	rm -rf bin
	rm -f tags
	rm -f cscope.out
	rm -f docs/paper.pdf
	rm -rf docs/reference
	rm -f docs/manual.pdf
FORCE:

# Clarifies that these are not files :-).
.PHONY: all run docs tags clean distclean
