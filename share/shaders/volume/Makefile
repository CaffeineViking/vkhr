all: volume.vert.spv volume.frag.spv voxelize.comp.spv

volume.vert.spv: volume.vert ../scene_graph/camera.glsl ../strand/strand.glsl volume.glsl ../strand/../volume/bounding_box.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../scene_graph/camera.glsl ../scene_graph/lights.glsl ../volume/volume_rendering.glsl ../volume/../utils/math.glsl ../volume/filter_volume.glsl ../volume/sample_volume.glsl ../shading_models/kajiya-kay.glsl ../strand/../volume/bounding_box.glsl ../volume/local_ambient_occlusion.glsl ../volume/raymarch.glsl volume.glsl ../strand/strand.glsl
	glslc -O -g -c volume.frag

voxelize.comp.spv: voxelize.comp sample_volume.glsl ../strand/strand.glsl ../strand/../volume/bounding_box.glsl bounding_box.glsl
	glslc -O -g -c voxelize.comp