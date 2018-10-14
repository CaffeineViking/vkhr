name = "vkhr"
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
	@echo "   download-premake"
	@echo "   download-glfw"
	@echo "   solution"
	@echo "   docs"
	@echo "   tags"
	@echo "   clean"
	@echo "   distclean"
	@echo ""

download: download-modules download-premake
download-modules: FORCE
	git submodule update --init --recursive --depth 1
download-premake: FORCE
	rm -f premake5.exe
	wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha12/premake-5.0.0-alpha12-windows.zip
	unzip premake-5.0.0-alpha12-windows.zip
	rm -f premake-5.0.0-alpha12-windows.zip

solution: FORCE
	premake5 vs2017

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
	rm -rf docs/reference
FORCE:

.PHONY: all run help solution download download-modules download-premake docs tags clean distclean
