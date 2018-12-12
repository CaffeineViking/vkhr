#include <vkhr/vulkan/billboard.hh>

#include <vkhr/camera.hh>
#include <vkhr/light_source.hh>
#include <vkhr/rasterizer.hh>

namespace vkhr {
    namespace vulkan {
        Billboard::Billboard(const vkhr::Billboard& billboards,
                             vkhr::Rasterizer& vulkan_renderer) {
            load(billboards, vulkan_renderer);
        }

        Billboard::Billboard(const std::uint32_t width, const std::uint32_t height,
                             vkhr::Rasterizer& vulkan_renderer, bool flip_image) {
            billboard_image = vk::DeviceImage {
                vulkan_renderer.device,
                width,
                height,
                vkhr::Image::get_expected_size(width, height)
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_image, VK_OBJECT_TYPE_IMAGE, "Billboard Image", id);

            billboard_view = vk::ImageView {
                vulkan_renderer.device,
                billboard_image
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_view, VK_OBJECT_TYPE_IMAGE_VIEW, "Billboard Image View", id);

            billboard_sampler = vk::Sampler { vulkan_renderer.device };

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_sampler, VK_OBJECT_TYPE_SAMPLER, "Billboard Sampler", id);

            ++id;
        }

        void Billboard::load(const vkhr::Billboard& billboard,
                             vkhr::Rasterizer& vulkan_renderer) {
            auto command_list = vulkan_renderer.command_pool.allocate_and_begin();
            billboard_image = vk::DeviceImage {
                vulkan_renderer.device,
                command_list,
                billboard.get_image()
            }; command_list.end();

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_image, VK_OBJECT_TYPE_IMAGE, "Billboard Image", id);

            vulkan_renderer.device.get_graphics_queue().submit(command_list)
                                                       .wait_idle();

            billboard_view = vk::ImageView {
                vulkan_renderer.device,
                billboard_image
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_view, VK_OBJECT_TYPE_IMAGE_VIEW, "Billboard Image View", id);

            billboard_sampler = vk::Sampler { vulkan_renderer.device };

            vk::DebugMarker::object_name(vulkan_renderer.device, billboard_sampler, VK_OBJECT_TYPE_SAMPLER, "Billboard Sampler", id);

            ++id;
        }

        void Billboard::send_img(vk::DescriptorSet& descriptor_set, vkhr::Image& img, vk::CommandBuffer& command_buffer) {
            billboard_image.staged_copy(img, command_buffer);
            update(descriptor_set, billboard_view, billboard_sampler);
        }

        void Billboard::update(vk::DescriptorSet& descriptor_set, vk::ImageView& image_view, vk::Sampler& image_sampler) {
            descriptor_set.write(1, image_view, image_sampler);
        }

        void Billboard::draw(vk::CommandBuffer& command_buffer) {
            command_buffer.draw(6, 1); // WARNING: this isn't how
            // billboard class should actually look like since it
            // won't perform very well. The proper way is to make
            // a storage buffer and instance draw the billboards!
        }

        void Billboard::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.enable_alpha_mix(0);

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("billboards.vert"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Billboard Vertex Shader");
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("billboards.frag"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[1],
                                       VK_OBJECT_TYPE_SHADER_MODULE, "Billboard Fragment Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                    { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER }
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
                                         "Billboards Descriptor Set Layout");

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Billboards Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.camera_vp[i]);
                // the combined image sampler descriptors can only written later.
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // model.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Billboards Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.color_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Billboards Graphics Pipeline");
        }

        glm::mat4 Billboard::Identity { 1.0f };
        int Billboard::id { 0 };
    }
}