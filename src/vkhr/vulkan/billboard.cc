#include <vkhr/vulkan/billboard.hh>

#include <vkhr/camera.hh>
#include <vkhr/light_source.hh>
#include <vkhr/rasterizer.hh>

namespace vkhr {
    namespace vulkan {
        Billboard::Billboard(const vkhr::Billboard& billboards,
                             vkhr::Rasterizer& vulkan_renderer,
                             Pipeline& billboard_gpu_pipeline)
                            : pipeline { &billboard_gpu_pipeline } {
            load(billboards, vulkan_renderer);
        }

        Billboard::Billboard(std::uint32_t width, std::uint32_t height,
                             Rasterizer& vulkan_renderer, Pipeline& pl)
                            : pipeline { &pl } {
            billboard_image = vk::DeviceImage {
                vulkan_renderer.device,
                width,
                height,
                vkhr::Image::get_expected_size(width, height)
            };

            billboard_view = vk::ImageView {
                vulkan_renderer.device,
                billboard_image
            };

            billboard_sampler = vk::Sampler { vulkan_renderer.device };
        }

        void Billboard::load(const vkhr::Billboard& billboards,
                             vkhr::Rasterizer& vulkan_renderer) {
            // TODO: case where an image is already provided...
        }

        void Billboard::update(MVP& transform, std::size_t i) {
            pipeline->descriptor_states[i].uniform_buffers[0].update(transform);
        }

        void Billboard::update(vkhr::Image& image, vk::CommandPool& command_pool) {
            billboard_image.staged_copy(image, command_pool);
        }

        void Billboard::draw(vk::CommandBuffer& command_list, std::size_t i) {
            pipeline->descriptor_sets[i].write(1, billboard_view, billboard_sampler);

            command_list.bind_pipeline(pipeline->pipeline);

            auto& descriptor_sets = pipeline->descriptor_sets[i];

            command_list.bind_descriptor_set(descriptor_sets,
                                             pipeline->pipeline);

            command_list.draw(6, 1); // WARNING: this isn't how a
            // billboard class should actually look like since it
            // won't perform very well. The proper way is to make
            // a storage buffer and instance draw the billboards!
        }

        void Billboard::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.enable_depth_test();
            pipeline.fixed_stages.enable_alpha_mix(0);

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("billboards.vert"));
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("billboards.frag"));

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                    { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
                }
            };

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout);
            pipeline.descriptor_states.resize(pipeline.descriptor_sets.size());
            for (std::size_t i = 0; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.write_uniform_buffer(i, 0, sizeof(MVP), vulkan_renderer.device);
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout
            };

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.render_pass
            };
        }
    }
}