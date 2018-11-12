all: billboards.vert.spv kajiya-kay.vert.spv billboards.frag.spv kajiya-kay.frag.spv

billboards.vert.spv: billboards.vert
	glslc -O -Os -c billboards.vert

kajiya-kay.vert.spv: kajiya-kay.vert
	glslc -O -Os -c kajiya-kay.vert

billboards.frag.spv: billboards.frag
	glslc -O -Os -c billboards.frag

kajiya-kay.frag.spv: kajiya-kay.frag kajiya-kay.glsl
	glslc -O -Os -c kajiya-kay.frag