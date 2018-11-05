all: strand.vert.spv strand.frag.spv

strand.vert.spv: strand.vert
	glslc -O -Os -c strand.vert

strand.frag.spv: strand.frag strand.glsl
	glslc -O -Os -c strand.frag