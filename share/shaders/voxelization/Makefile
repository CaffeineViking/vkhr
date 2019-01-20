all: voxelize_vertices.comp.spv

voxelize_vertices.comp.spv: voxelize_vertices.comp aabb.glsl ../strand/strand.glsl ../strand/../voxelization/aabb.glsl sample_volume.glsl ../math.glsl
	glslc -O -g -c voxelize_vertices.comp