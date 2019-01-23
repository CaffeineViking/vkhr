all: volume.vert.spv volume.frag.spv

volume.vert.spv: volume.vert ../scene_graph/camera.glsl ../strand/strand.glsl volume.glsl ../strand/../volume/bounding_box.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../scene_graph/camera.glsl ../strand/strand.glsl volume.glsl ../scene_graph/lights.glsl ../strand/../volume/bounding_box.glsl ../volume/ambient_occlusion.glsl ../volume/raymarch.glsl ../volume/sample_volume.glsl
	glslc -O -g -c volume.frag