all: billboards.vert.spv depth_pass.vert.spv strands.vert.spv billboards.frag.spv strands.frag.spv

billboards.vert.spv: billboards.vert camera.glsl
	glslc -O -g -c billboards.vert

depth_pass.vert.spv: depth_pass.vert
	glslc -O -g -c depth_pass.vert

strands.vert.spv: strands.vert camera.glsl
	glslc -O -g -c strands.vert

billboards.frag.spv: billboards.frag
	glslc -O -g -c billboards.frag

strands.frag.spv: strands.frag camera.glsl lights.glsl shadow_map.glsl kajiya-kay.glsl math.glsl
	glslc -O -g -c strands.frag