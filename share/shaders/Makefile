all: billboards.vert.spv depth_pass.vert.spv models.vert.spv strands.vert.spv billboards.frag.spv models.frag.spv strands.frag.spv

billboards.vert.spv: billboards.vert camera.glsl
	glslc -O -g -c billboards.vert

depth_pass.vert.spv: depth_pass.vert
	glslc -O -g -c depth_pass.vert

models.vert.spv: models.vert camera.glsl
	glslc -O -g -c models.vert

strands.vert.spv: strands.vert camera.glsl
	glslc -O -g -c strands.vert

billboards.frag.spv: billboards.frag
	glslc -O -g -c billboards.frag

models.frag.spv: models.frag camera.glsl blinn-phong.glsl lights.glsl shadow_map.glsl math.glsl
	glslc -O -g -c models.frag

strands.frag.spv: strands.frag camera.glsl lights.glsl shadow_map.glsl kajiya-kay.glsl math.glsl
	glslc -O -g -c strands.frag