#ifndef VKHR_VULKAN_DEPTH_MAP_HH
#define VKHR_VULKAN_DEPTH_MAP_HH

#include <vkhr/vulkan/pipeline.hh>
#include <vkhr/vulkan/drawable.hh>

#include <vkpp/framebuffer.hh>
#include <vkpp/image.hh>
#include <vkpp/sampler.hh>

#include <cstdint>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class DepthView final : public Drawable {
        public:
            DepthView() = default;
            DepthView(const std::uint32_t width, const std::uint32_t height,
                      Rasterizer& vulkan_renderer, Pipeline& depth_pipeline);

            void draw(vk::CommandBuffer& command_buffer,
                      std::size_t framebuffer) override;

            VkFormat      get_attachment_format();
            VkImageLayout get_read_depth_layout();
            VkImageLayout get_attachment_layout();

            Pipeline* pipeline { nullptr };

            static void build_pipeline(Pipeline& pipeline_reference,
                                       Rasterizer& vulkan_renderer);

        private:
            vk::Image image;
            vk::DeviceMemory memory;
            vk::ImageView image_view;
            vk::Framebuffer framebuffer;
            vk::Sampler sampler;
        };
    }
}

#endif