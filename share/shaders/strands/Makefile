all: strand.vert.spv strand.geom.spv strand.frag.spv

strand.vert.spv: strand.vert strand.glsl ../scene_graph/camera.glsl ../volumes/bounding_box.glsl
	glslc -O -g -c strand.vert

strand.geom.spv: strand.geom strand.glsl ../scene_graph/camera.glsl ../volumes/bounding_box.glsl
	glslc -O -g -c strand.geom

strand.frag.spv: strand.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../volumes/local_ambient_occlusion.glsl ../self-shadowing/../volumes/../utils/math.glsl ../shading/kajiya-kay.glsl ../self-shadowing/approximate_deep_shadows.glsl ../self-shadowing/../utils/math.glsl ../self-shadowing/../volumes/sample_volume.glsl ../self-shadowing/tex2Dproj.glsl ../volumes/sample_volume.glsl ../transparency/ppll.glsl strand.glsl ../anti-aliasing/gbaa.glsl ../scene_graph/lights.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/params.glsl ../volumes/bounding_box.glsl
	glslc -O -g -c strand.frag