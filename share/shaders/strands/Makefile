all: strand.vert.spv strand.geom.spv strand.frag.spv

strand.vert.spv: strand.vert ../volumes/bounding_box.glsl ../scene_graph/camera.glsl strand.glsl
	glslc -O -g -c strand.vert

strand.geom.spv: strand.geom ../volumes/bounding_box.glsl ../scene_graph/camera.glsl strand.glsl
	glslc -O -g -c strand.geom

strand.frag.spv: strand.frag ../volumes/bounding_box.glsl strand.glsl ../scene_graph/params.glsl ../self-shadowing/../utils/math.glsl ../self-shadowing/../volumes/sample_volume.glsl ../self-shadowing/tex2Dproj.glsl ../anti-aliasing/gpaa.glsl ../self-shadowing/approximate_deep_shadows.glsl ../scene_graph/camera.glsl ../shading/kajiya-kay.glsl ../volumes/local_ambient_occlusion.glsl ../self-shadowing/../volumes/../utils/math.glsl ../self-shadowing/linearize_depth.glsl ../volumes/sample_volume.glsl ../level_of_detail/../scene_graph/params.glsl ../transparency/ppll.glsl ../level_of_detail/scheme.glsl ../scene_graph/lights.glsl ../scene_graph/shadow_maps.glsl
	glslc -O -g -c strand.frag