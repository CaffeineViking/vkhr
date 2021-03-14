name   = "vkhr"
config = "release"

all: shaders program
run: all
	bin/${name} ${args}
benchmark: all
	bin/${name} ${args} --benchmark yes

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
	@echo "   benchmark"
	@echo "   help"
	@echo "   shaders"
	@echo "   program"
	@echo "   download"
	@echo "   download-modules"
	@echo "   pre-generate"
	@echo "   solution"
	@echo "   bundle-assets"
	@echo "   archive-build"
	@echo "   distribute"
	@echo "   docs"
	@echo "   tags"
	@echo "   clean"
	@echo "   distclean"
	@echo ""

program: FORCE
	premake5 gmake
	make -j8 -C build config=${config}

shaders: FORCE
	@-utils/glslc.py share/shaders/billboards
	@-make --no-print-directory -C share/shaders/billboards
	@-utils/glslc.py share/shaders/models
	@-make --no-print-directory -C share/shaders/models
	@-utils/glslc.py share/shaders/self-shadowing
	@-make --no-print-directory -C share/shaders/self-shadowing
	@-utils/glslc.py share/shaders/transparency
	@-make --no-print-directory -C share/shaders/transparency
	@-utils/glslc.py share/shaders/strands
	@-make --no-print-directory -C share/shaders/strands
	@-utils/glslc.py share/shaders/volumes
	@-make --no-print-directory -C share/shaders/volumes

download: download-modules
download-modules: FORCE
	git submodule update --init --recursive --depth 1

pre-generate: clean solution
	rm -rf build/.vs
	git add -f build
solution: FORCE
	premake5 vs2019

bundle-assets: FORCE
	rm -rf bin/share
	cp -r share bin/share

archive-build: FORCE
	mv bin ${name}
	zip -r ${name} ${name}
	mv ${name} bin

distribute: bundle-assets archive-build

docs: FORCE
	make -C docs
tags: FORCE
	ctags  -R     src/${name}    src/vkpp    include/${name}    include/vkpp
	cscope -Rb -s src/${name} -s src/vkpp -s include/${name} -s include/vkpp

clean: FORCE
	rm -rf build/obj
	rm -f  build/Makefile
	rm -f  build/${name}.make
	rm -rf docs/build
distclean: clean
	rm -f ${name}.zip
	rm -f cscope.out
	rm -f tags
	rm -f imgui.ini
	rm -rf bin/share
	rm -rf build/.vs
	find bin/ -type f ! \( -name "*.dll" -o -name "*.ico" \) -delete
FORCE:

.PHONY: all run benchmark help program shaders download download-modules pre-generate solution bundle-assets distribute docs tags clean distclean
