all: volume.vert.spv volume.frag.spv voxelize.comp.spv

volume.vert.spv: volume.vert ../scene_graph/camera.glsl ../strand/strand.glsl volume.glsl ../strand/../volume/bounding_box.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../scene_graph/lights.glsl ../volume/volume_rendering.glsl ../volume/sample_volume.glsl ../strand/../volume/bounding_box.glsl ../volume/local_ambient_occlusion.glsl ../self-shadowing/approximate_deep_shadows.glsl ../self-shadowing/../volume/sample_volume.glsl ../self-shadowing/../utils/math.glsl ../volume/filter_volume.glsl ../self-shadowing/tex2Dproj.glsl ../shading_models/kajiya-kay.glsl ../volume/raymarch.glsl ../volume/../utils/math.glsl ../scene_graph/params.glsl volume.glsl ../strand/strand.glsl
	glslc -O -g -c volume.frag

voxelize.comp.spv: voxelize.comp sample_volume.glsl ../strand/strand.glsl ../strand/../volume/bounding_box.glsl bounding_box.glsl
	glslc -O -g -c voxelize.comp