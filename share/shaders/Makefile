all: simple.vert.spv simple.frag.spv

simple.vert.spv: simple.vert simple.glsl
	glslc -O -Os -c -o spirv/simple.vert.spv simple.vert

simple.frag.spv: simple.frag simple.glsl
	glslc -O -Os -c -o spirv/simple.frag.spv simple.frag