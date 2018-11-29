#include <vkhr/vulkan/depth_map.hh>

#include <vkhr/rasterizer.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        DepthView::DepthView(const std::uint32_t width, const std::uint32_t height,
                             Rasterizer& vulkan_renderer, Pipeline& depth_pipeline) {
            image = vk::Image {
                vulkan_renderer.device,
                width, height,
                VK_FORMAT_D32_SFLOAT,
                VK_IMAGE_USAGE_SAMPLED_BIT |
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
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

            sampler = vk::Sampler {
                vulkan_renderer.device,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, sampler, VK_OBJECT_TYPE_SAMPLER, "Depth Map Sampler");
        }

        void DepthView::draw(vk::CommandBuffer& command_buffer, std::size_t i) {
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

        void DepthView::build_pipeline(Pipeline& pipeline_reference,
                                       Rasterizer& vulkan_renderer) {
        }
    }
}
