all: volume.vert.spv volume.frag.spv voxelize.comp.spv

volume.vert.spv: volume.vert ../strands/../volumes/bounding_box.glsl ../strands/strand.glsl ../scene_graph/camera.glsl volume.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../strands/strand.glsl volume.glsl ../scene_graph/params.glsl ../self-shadowing/../utils/math.glsl ../self-shadowing/../volumes/../utils/math.glsl ../strands/../volumes/bounding_box.glsl ../self-shadowing/approximate_deep_shadows.glsl ../shading/kajiya-kay.glsl ../self-shadowing/../volumes/sample_volume.glsl ../self-shadowing/tex2Dproj.glsl ../self-shadowing/linearize_depth.glsl ../level_of_detail/../scene_graph/params.glsl ../transparency/ppll.glsl ../level_of_detail/scheme.glsl raymarch.glsl sample_volume.glsl ../scene_graph/lights.glsl volume_rendering.glsl ../scene_graph/camera.glsl local_ambient_occlusion.glsl
	glslc -O -g -c volume.frag

voxelize.comp.spv: voxelize.comp ../strands/../volumes/bounding_box.glsl bounding_box.glsl sample_volume.glsl ../strands/strand.glsl ../utils/math.glsl
	glslc -O -g -c voxelize.comp