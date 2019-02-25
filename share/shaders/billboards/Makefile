all: billboard.vert.spv billboard.frag.spv

billboard.vert.spv: billboard.vert ../scene_graph/camera.glsl billboard.glsl
	glslc -O -g -c billboard.vert

billboard.frag.spv: billboard.frag
	glslc -O -g -c billboard.frag