all: volume.vert.spv volume.frag.spv voxelize.comp.spv

volume.vert.spv: volume.vert ../scene_graph/camera.glsl ../strands/../volumes/bounding_box.glsl volume.glsl ../strands/strand.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../scene_graph/lights.glsl ../self-shadowing/approximate_deep_shadows.glsl ../self-shadowing/../utils/math.glsl ../self-shadowing/../volumes/sample_volume.glsl ../shading/kajiya-kay.glsl ../self-shadowing/../volumes/../utils/math.glsl sample_volume.glsl ../self-shadowing/tex2Dproj.glsl raymarch.glsl local_ambient_occlusion.glsl volume_rendering.glsl ../scene_graph/params.glsl volume.glsl ../strands/strand.glsl ../strands/../volumes/bounding_box.glsl
	glslc -O -g -c volume.frag

voxelize.comp.spv: voxelize.comp ../utils/math.glsl sample_volume.glsl ../strands/strand.glsl ../strands/../volumes/bounding_box.glsl bounding_box.glsl
	glslc -O -g -c voxelize.comp