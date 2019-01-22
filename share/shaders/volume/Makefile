all: volume.vert.spv volume.frag.spv

volume.vert.spv: volume.vert ../scene_graph/camera.glsl ../strand/strand.glsl volume.glsl ../strand/../volume/bounding_box.glsl
	glslc -O -g -c volume.vert

volume.frag.spv: volume.frag ../scene_graph/camera.glsl ../strand/../volume/bounding_box.glsl ../volume/local_ambient_occlusion.glsl ../shading_models/kajiya-kay.glsl ../volume/sample_volume.glsl ../strand/strand.glsl volume.glsl ../scene_graph/lights.glsl
	glslc -O -g -c volume.frag