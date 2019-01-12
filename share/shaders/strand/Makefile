all: strand.vert.spv strand.frag.spv

strand.vert.spv: strand.vert strand.glsl ../scene_graph/camera.glsl ../voxelization/aabb.glsl
	glslc -O -g -c strand.vert

strand.frag.spv: strand.frag ../self-shadowing/linearize_depth.glsl ../scene_graph/camera.glsl ../voxelization/aabb.glsl ../self-shadowing/../math.glsl ../self-shadowing/approximate_deep_shadows.glsl ../shading_models/kajiya-kay.glsl ../self-shadowing/tex2Dproj.glsl ../scene_graph/shadow_maps.glsl ../scene_graph/lights.glsl ../voxelization/raymarch.glsl ../voxelization/sample_volume.glsl ../voxelization/../math.glsl ../scene_graph/params.glsl strand.glsl
	glslc -O -g -c strand.frag