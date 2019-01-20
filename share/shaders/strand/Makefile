all: strand.vert.spv strand.geom.spv strand.frag.spv

strand.vert.spv: strand.vert strand.glsl ../scene_graph/camera.glsl ../voxelization/aabb.glsl
	glslc -O -g -c strand.vert

strand.geom.spv: strand.geom strand.glsl ../scene_graph/camera.glsl ../voxelization/aabb.glsl
	glslc -O -g -c strand.geom

strand.frag.spv: strand.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../voxelization/aabb.glsl ../self-shadowing/../math.glsl ../self-shadowing/approximate_deep_shadows.glsl ../shading_models/kajiya-kay.glsl ../voxelization/../math.glsl ../self-shadowing/tex2Dproj.glsl ../voxelization/local_ambient_occlusion.glsl ../voxelization/sample_volume.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/lights.glsl ../scene_graph/params.glsl strand.glsl
	glslc -O -g -c strand.frag