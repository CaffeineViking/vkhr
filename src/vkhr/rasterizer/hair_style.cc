#include <vkhr/rasterizer/hair_style.hh>

#include <vkhr/rasterizer.hh>

#include <vkhr/scene_graph/camera.hh>
#include <vkhr/scene_graph/light_source.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        HairStyle::HairStyle(const vkhr::HairStyle& hair_style,
                             vkhr::Rasterizer& vulkan_renderer) {
            load(hair_style, vulkan_renderer);
        }

        void HairStyle::load(const vkhr::HairStyle& hair_style,
                             vkhr::Rasterizer& vulkan_renderer) {
            vertices = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_vertices()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, vertices, VK_OBJECT_TYPE_BUFFER, "Hair Position Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, vertices.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Position Device Memory", id);

            tangents = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_tangents()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangents, VK_OBJECT_TYPE_BUFFER, "Hair Tangent Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, tangents.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Tangent Device Memory", id);

            thickness = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_thickness()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, thickness, VK_OBJECT_TYPE_BUFFER, "Hair Thickness Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, thickness.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Thickness Device Memory", id);

            segments = vk::IndexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_indices()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, segments, VK_OBJECT_TYPE_BUFFER, "Hair Index Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, segments.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Index Device Memory", id);

            parameters.hair_shininess = 50.0f; // Using Kajiya-Kay.
            parameters.strand_radius = hair_style.get_default_thickness();
            parameters.hair_opacity = hair_style.get_default_transparency();
            parameters.strand_ratio = 1.00f; // i.e. don't reduce strands.
            parameters.hair_color = hair_style.get_default_color();

            parameters.volume_resolution = glm::vec3 { 256,256,256 };
            parameters.volume_bounds = hair_style.get_bounding_box();

            parameter_buffer = vk::UniformBuffer {
                vulkan_renderer.device,
                parameters
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, parameter_buffer, VK_OBJECT_TYPE_BUFFER, "Hair Parameters Buffer", id);

            auto strand_volume = hair_style.voxelize_segments(256, 256, 256);

            strand_volume.normalize();

            density_sampler = vk::Sampler {
                vulkan_renderer.device,
                VK_FILTER_LINEAR,      VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, density_sampler, VK_OBJECT_TYPE_SAMPLER, "Hair Density Sampler", id);

            VkDeviceSize density_length = parameters.volume_resolution.x *
                                          parameters.volume_resolution.y * 
                                          parameters.volume_resolution.z *
                                          sizeof(unsigned char); // bytes.

            density_volume = vk::DeviceImage {
                vulkan_renderer.device,
                static_cast<std::uint32_t>(parameters.volume_resolution.x),
                static_cast<std::uint32_t>(parameters.volume_resolution.y),
                static_cast<std::uint32_t>(parameters.volume_resolution.z),
                vulkan_renderer.command_pool,
                strand_volume.densities
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, density_volume, VK_OBJECT_TYPE_IMAGE, "Hair Density Volume", id);

            density_view = vk::ImageView {
                vulkan_renderer.device,
                density_volume
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, density_view, VK_OBJECT_TYPE_IMAGE_VIEW, "Hair Density View", id);

            tangent_sampler = vk::Sampler {
                vulkan_renderer.device,
                VK_FILTER_LINEAR,      VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangent_sampler, VK_OBJECT_TYPE_SAMPLER, "Hair Tangent Sampler", id);

            tangent_volume = vk::DeviceImage {
                vulkan_renderer.device,
                static_cast<std::uint32_t>(parameters.volume_resolution.x),
                static_cast<std::uint32_t>(parameters.volume_resolution.y),
                static_cast<std::uint32_t>(parameters.volume_resolution.z),
                vulkan_renderer.command_pool,
                strand_volume.tangents
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangent_volume, VK_OBJECT_TYPE_IMAGE, "Hair Tangent Volume", id);

            tangent_view = vk::ImageView {
                vulkan_renderer.device,
                tangent_volume
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangent_view, VK_OBJECT_TYPE_IMAGE_VIEW, "Hair Tangent View", id);

            volume = Volume {
                *this,
                vulkan_renderer
            };

            ++id;
        }

        void HairStyle::voxelize(Pipeline& voxel_pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
            density_volume.transition(command_buffer,
                                      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_IMAGE_LAYOUT_GENERAL,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            // command_buffer.clear_color_image(density_volume, { /* 0 */ });

            density_volume.transition(command_buffer,
                                      VK_ACCESS_TRANSFER_WRITE_BIT,
                                      VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_GENERAL,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

            descriptor_set.write(0, vertices);
            descriptor_set.write(2, parameter_buffer);
            descriptor_set.write(3, density_view);

            command_buffer.bind_descriptor_set(descriptor_set, voxel_pipeline);
            command_buffer.dispatch((vertices.count()*parameters.strand_ratio) / 512);
        }

        void HairStyle::draw_volume(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
            volume.set_current_volume(density_view, tangent_view);
            volume.set_volume_parameters(parameter_buffer);
            volume.set_volume_sampler(density_sampler, tangent_sampler);
            volume.draw(pipeline, descriptor_set, command_buffer);
        }

        void HairStyle::draw(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
            if (descriptor_set.get_layout().get_bindings().size()) {
                descriptor_set.write(2, parameter_buffer);
                descriptor_set.write(3, density_view, density_sampler);
            }

            command_buffer.set_line_width(parameters.strand_radius);

            command_buffer.bind_descriptor_set(descriptor_set, pipeline);

            command_buffer.bind_vertex_buffer(0, vertices,  0);
            command_buffer.bind_vertex_buffer(1, tangents,  0);
            command_buffer.bind_vertex_buffer(2, thickness, 0);

            command_buffer.bind_index_buffer(segments);

            command_buffer.draw_indexed(segments.count() * parameters.strand_ratio);
        }

        void HairStyle::update_parameters() {
            parameter_buffer.update(parameters);
        }

        void HairStyle::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) });
            pipeline.fixed_stages.add_vertex_binding({ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) });
            pipeline.fixed_stages.add_vertex_binding({ 2, 2, VK_FORMAT_R32_SFLOAT,       sizeof(float)     });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH);

            pipeline.fixed_stages.set_line_width(1.0);
            pipeline.fixed_stages.enable_alpha_blending_for(0);
            pipeline.fixed_stages.enable_depth_test(false);

            std::uint32_t light_count = vulkan_renderer.shadow_maps.size();

            struct Constants {
                std::uint32_t light_size;
            } constant_data {
                light_count
            };

            std::vector<VkSpecializationMapEntry> constants {
                { 0, 0, sizeof(std::uint32_t) } // light size
            };

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("strands/strand.vert"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0], VK_OBJECT_TYPE_SHADER_MODULE, "Hair Vertex Shader");
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("strands/strand.frag"), constants, &constant_data, sizeof(constant_data));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[1], VK_OBJECT_TYPE_SHADER_MODULE, "Hair Fragment Shader");

            std::vector<vk::DescriptorSet::Binding> descriptor_bindings {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
                { 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
                { 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
                { 7, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 8, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER }
            };

            for (std::uint32_t i { 0 }; i < light_count; ++i)
                descriptor_bindings.push_back({ 9 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device, descriptor_bindings
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Descriptor Set Layout");

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.camera[i]);
                pipeline.descriptor_sets[i].write(1, vulkan_renderer.lights[i]);
                pipeline.descriptor_sets[i].write(4, vulkan_renderer.params[i]);

                pipeline.descriptor_sets[i].write(5, vulkan_renderer.ppll.get_heads_view());
                pipeline.descriptor_sets[i].write(6, vulkan_renderer.ppll.get_nodes());
                pipeline.descriptor_sets[i].write(7, vulkan_renderer.ppll.get_parameters());
                pipeline.descriptor_sets[i].write(8, vulkan_renderer.ppll.get_node_counter());

                for (std::uint32_t j { 0 }; j < light_count; ++j)
                    pipeline.descriptor_sets[i].write(9 + j, vulkan_renderer.shadow_maps[j].get_image_view(),
                                                      vulkan_renderer.shadow_maps[j].get_sampler());
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // model.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Hair Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.color_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Hair Graphics Pipeline");
        }

        void HairStyle::depth_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) });

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH);
            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);

            pipeline.fixed_stages.set_culling_mode(VK_CULL_MODE_BACK_BIT);

            pipeline.fixed_stages.set_line_width(1.0);
            pipeline.fixed_stages.enable_depth_test();

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("self-shadowing/depth_map.vert"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0], VK_OBJECT_TYPE_SHADER_MODULE, "Hair Depth Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Depth Descriptor Set Layout");

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Depth Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // transforms.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Hair Depth Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.depth_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Hair Depth Graphics Pipeline");
        }

        void HairStyle::voxel_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("volumes/voxelize.comp"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Hair Voxelization Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
                    { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                    { 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE  }
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Voxel Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Voxel Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Hair Voxel Pipeline Layout");

            pipeline.compute_pipeline = vk::ComputePipeline {
                vulkan_renderer.device,
                pipeline.shader_stages[0],
                pipeline.pipeline_layout
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.compute_pipeline,
                                         VK_OBJECT_TYPE_PIPELINE, "Hair Voxel Pipeline");
        }

        void HairStyle::reduce(float ratio) {
            parameters.strand_ratio = ratio;
        }

        std::size_t HairStyle::get_geometry_size() const {
            return segments.get_size() +
                   vertices.get_size() +
                   tangents.get_size() +
                   thickness.get_size();
        }

        std::size_t HairStyle::get_volume_size() const {
            return density_volume.get_memory_requirements().size +
                   tangent_volume.get_memory_requirements().size;
        }

        int HairStyle::id { 0 };
    }
}
