all: billboard.vert.spv depth_pass.vert.spv model.vert.spv strand.vert.spv billboard.frag.spv model.frag.spv strand.frag.spv voxelize.comp.spv

billboard.vert.spv: billboard.vert camera.glsl
	glslc -O -g -c billboard.vert

depth_pass.vert.spv: depth_pass.vert
	glslc -O -g -c depth_pass.vert

model.vert.spv: model.vert camera.glsl
	glslc -O -g -c model.vert

strand.vert.spv: strand.vert strand.glsl bounding_box.glsl camera.glsl
	glslc -O -g -c strand.vert

billboard.frag.spv: billboard.frag
	glslc -O -g -c billboard.frag

model.frag.spv: model.frag camera.glsl filter_shadows.glsl blinn-phong.glsl tex2Dproj.glsl linearize_depth.glsl math.glsl light.glsl shadow_map.glsl parameters.glsl
	glslc -O -g -c model.frag

strand.frag.spv: strand.frag approximate_deep_shadows.glsl camera.glsl shadow_map.glsl kajiya-kay.glsl volume.glsl light.glsl math.glsl tex2Dproj.glsl linearize_depth.glsl parameters.glsl strand.glsl bounding_box.glsl
	glslc -O -g -c strand.frag

voxelize.comp.spv: voxelize.comp volume.glsl strand.glsl bounding_box.glsl
	glslc -O -g -c voxelize.comp