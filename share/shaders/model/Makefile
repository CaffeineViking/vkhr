all: model.vert.spv model.frag.spv

model.vert.spv: model.vert ../scene_graph/camera.glsl
	glslc -O -g -c model.vert

model.frag.spv: model.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../shading_models/blinn-phong.glsl ../self-shadowing/tex2Dproj.glsl ../self-shadowing/filter_shadows.glsl ../self-shadowing/../math.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/lights.glsl ../scene_graph/params.glsl model.glsl
	glslc -O -g -c model.frag