all: voxelize.comp.spv

voxelize.comp.spv: voxelize.comp aabb.glsl ../strand/strand.glsl ../strand/../voxelization/aabb.glsl sample_volume.glsl ../math.glsl
	glslc -O -g -c voxelize.comp