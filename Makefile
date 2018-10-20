name   = "vkhr"
config = "debug"

all: FORCE
	premake5 gmake
	make -j8 -C build config=${config}
run: all
	bin/${name} ${args}

help: FORCE
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  release"
	@echo "  debug"
	@echo ""
	@echo "TARGETS:"
	@echo "   all"
	@echo "   run"
	@echo "   help"
	@echo "   download (all)"
	@echo "   download-modules"
	@echo "   pre-generate"
	@echo "   solution"
	@echo "   docs"
	@echo "   tags"
	@echo "   clean"
	@echo "   distclean"
	@echo ""

download: download-modules
download-modules: FORCE
	git submodule update --init --recursive --depth 1

pre-generate: clean solution
	rm -f build/Makefile
	rm -f build/vkhr.make
	git add -f build
solution: FORCE
	premake5 vs2017

docs: FORCE
	make -C docs
tags: FORCE
	ctags  -R     src/${name}    include/${name}
	cscope -Rb -s src/${name} -s include/${name}

clean: FORCE
	rm -rf build/obj
	rm -rf docs/build
distclean: clean
	rm -f  cscope.out
	rm -f  tags
	rm -rf bin
FORCE:

.PHONY: all run help download download-modules pre-generate solution docs tags clean distclean
