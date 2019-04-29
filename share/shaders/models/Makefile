all: model.vert.spv model.frag.spv

model.vert.spv: model.vert ../scene_graph/camera.glsl
	glslc -O -g -c model.vert

model.frag.spv: model.frag model.glsl ../scene_graph/lights.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/params.glsl ../self-shadowing/../utils/rand.glsl ../self-shadowing/filter_shadows.glsl ../self-shadowing/linearize_depth.glsl ../shading/lambertian.glsl ../self-shadowing/tex2Dproj.glsl ../scene_graph/camera.glsl ../self-shadowing/../scene_graph/params.glsl
	glslc -O -g -c model.frag