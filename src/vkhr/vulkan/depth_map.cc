#include <vkhr/vulkan/depth_map.hh>

#include <vkhr/rasterizer.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        DepthView::DepthView(const std::uint32_t width, const std::uint32_t height,
                             const std::uint32_t depth, Rasterizer& vulkan_renderer) {
            image = vk::Image {
                vulkan_renderer.device,
                width, height,
                get_attachment_format(),
                VK_IMAGE_USAGE_SAMPLED_BIT |
                get_image_usage_flags()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, image, VK_OBJECT_TYPE_IMAGE, "Depth Map Image");

            memory = vk::DeviceMemory {
                vulkan_renderer.device,
                image.get_memory_requirements(),
                vk::DeviceMemory::Type::DeviceLocal
            };

            image.bind(memory);

            vk::DebugMarker::object_name(vulkan_renderer.device, memory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Depth Map Device Memory");

            image_view = vk::ImageView {
                vulkan_renderer.device,
                image
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, image_view, VK_OBJECT_TYPE_IMAGE_VIEW, "Depth Map Image View");

            framebuffer = vk::Framebuffer {
                vulkan_renderer.device,
                vulkan_renderer.depth_pass,
                image_view, VkExtent2D {
                    width, height
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, "Depth Map Framebuffer");

            sampler = vk::Sampler {
                vulkan_renderer.device,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, sampler, VK_OBJECT_TYPE_SAMPLER, "Depth Map Sampler");

            viewport = VkViewport {
                0.0f, 0.0f,
                static_cast<float>(width),
                static_cast<float>(height),
                0.0f, 1.0f
            };

            scissor = VkRect2D {
                { 0, 0 },
                { width, height }
            };
        }

        void DepthView::update_dynamic_viewport_scissor_depth(vk::CommandBuffer& command_list) {
            command_list.set_viewport(viewport);
            command_list.set_scissor(scissor);
        }

        vk::Framebuffer& DepthView::frame() {
            return framebuffer;
        }

        VkImageLayout DepthView::get_read_depth_layout() {
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        }

        VkFormat DepthView::get_attachment_format() {
            return VK_FORMAT_D32_SFLOAT;
        }

        VkImageLayout DepthView::get_attachment_layout() {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkImageUsageFlags DepthView::get_image_usage_flags() {
            return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        void DepthView::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)});

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);

            pipeline.fixed_stages.set_culling_mode(VK_CULL_MODE_BACK_BIT);

            pipeline.fixed_stages.set_line_width(1.0);
            pipeline.fixed_stages.enable_depth_test();

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("depth-pass.vert"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Depth Map Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Depth Map Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Depth Map Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.transform[i]);
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Depth Map Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.depth_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Depth Map Graphics Pipeline");
        }
    }
}
