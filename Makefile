name   = "vkhr"
config = "debug"

all: shaders program
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
	@echo "   shaders"
	@echo "   program"
	@echo "   download (all)"
	@echo "   download-modules"
	@echo "   pre-generate"
	@echo "   solution"
	@echo "   bundle-assets"
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
	@-utils/glslc.py share/shaders/billboard
	@-make --no-print-directory -C share/shaders/billboard
	@-utils/glslc.py share/shaders/BezierDirect
	@-make --no-print-directory -C share/shaders/BezierDirect
	@-utils/glslc.py share/shaders/model
	@-make --no-print-directory -C share/shaders/model
	@-utils/glslc.py share/shaders/self-shadowing
	@-make --no-print-directory -C share/shaders/self-shadowing
	@-utils/glslc.py share/shaders/strand
	@-make --no-print-directory -C share/shaders/strand
	@-utils/glslc.py share/shaders/volume
	@-make --no-print-directory -C share/shaders/volume
	@-utils/glslc.py share/shaders/volume/voxelization
	@-make --no-print-directory -C share/shaders/volume/voxelization

download: download-modules
download-modules: FORCE
	git submodule update --init --recursive --depth 1

pre-generate: clean solution
	rm -rf build/.vs
	git add -f build
solution: FORCE
	premake5 vs2017

bundle-assets: FORCE
	rm -rf bin/share
	cp -r share bin/share

distribute: bundle-assets
	mv bin ${name}
	zip -r ${name} ${name}
	mv ${name} bin

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
	find bin/ -type f ! -name "*.dll" -delete
FORCE:

.PHONY: all run help program shaders download download-modules pre-generate solution bundle-assets distribute docs tags clean distclean
