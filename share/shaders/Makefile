all: billboards.vert.spv depth_pass.vert.spv kajiya-kay.vert.spv billboards.frag.spv kajiya-kay.frag.spv

billboards.vert.spv: billboards.vert
	glslc -O -g -c billboards.vert

depth_pass.vert.spv: depth_pass.vert
	glslc -O -g -c depth_pass.vert

kajiya-kay.vert.spv: kajiya-kay.vert
	glslc -O -g -c kajiya-kay.vert

billboards.frag.spv: billboards.frag
	glslc -O -g -c billboards.frag

kajiya-kay.frag.spv: kajiya-kay.frag shadow_map.glsl light_info.glsl kajiya-kay.glsl
	glslc -O -g -c kajiya-kay.frag