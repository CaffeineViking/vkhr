all: voxelize_vertices.comp.spv

voxelize_vertices.comp.spv: voxelize_vertices.comp ../sample_volume.glsl ../bounding_box.glsl ../../strand/strand.glsl ../../strand/../volume/bounding_box.glsl
	glslc -O -g -c voxelize_vertices.comp