all: billboards.vert.spv depth-pass.vert.spv kajiya-kay.vert.spv billboards.frag.spv kajiya-kay.frag.spv

billboards.vert.spv: billboards.vert
	glslc -O -g -c billboards.vert

depth-pass.vert.spv: depth-pass.vert
	glslc -O -g -c depth-pass.vert

kajiya-kay.vert.spv: kajiya-kay.vert
	glslc -O -g -c kajiya-kay.vert

billboards.frag.spv: billboards.frag
	glslc -O -g -c billboards.frag

kajiya-kay.frag.spv: kajiya-kay.frag light_data.glsl kajiya-kay.glsl
	glslc -O -g -c kajiya-kay.frag