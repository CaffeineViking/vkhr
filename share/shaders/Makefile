all: billboards.vert.spv depth_pass.vert.spv kajiya-kay.vert.spv billboards.frag.spv kajiya-kay.frag.spv

billboards.vert.spv: billboards.vert camera.glsl
	glslc -O -g -c billboards.vert

depth_pass.vert.spv: depth_pass.vert
	glslc -O -g -c depth_pass.vert

kajiya-kay.vert.spv: kajiya-kay.vert camera.glsl
	glslc -O -g -c kajiya-kay.vert

billboards.frag.spv: billboards.frag
	glslc -O -g -c billboards.frag

kajiya-kay.frag.spv: kajiya-kay.frag camera.glsl lights.glsl shadow_map.glsl kajiya-kay.glsl math.h.glsl
	glslc -O -g -c kajiya-kay.frag