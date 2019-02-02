all: strand.vert.spv strand.geom.spv strand.frag.spv

strand.vert.spv: strand.vert strand.glsl ../scene_graph/camera.glsl ../volume/bounding_box.glsl
	glslc -O -g -c strand.vert

strand.geom.spv: strand.geom strand.glsl ../scene_graph/camera.glsl ../volume/bounding_box.glsl
	glslc -O -g -c strand.geom

strand.frag.spv: strand.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../self-shadowing/approximate_deep_shadows.glsl ../volume/local_ambient_occlusion.glsl ../shading_models/kajiya-kay.glsl ../self-shadowing/../volume/sample_volume.glsl ../volume/bounding_box.glsl ../self-shadowing/../utils/math.glsl ../volume/sample_volume.glsl ../self-shadowing/tex2Dproj.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/lights.glsl ../scene_graph/params.glsl strand.glsl
	glslc -O -g -c strand.frag