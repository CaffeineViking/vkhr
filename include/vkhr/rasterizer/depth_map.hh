#ifndef VKHR_VULKAN_DEPTH_VIEW_HH
#define VKHR_VULKAN_DEPTH_VIEW_HH

#include <vkhr/rasterizer/pipeline.hh>
#include <vkhr/rasterizer/drawable.hh>

#include <vkhr/scene_graph/light_source.hh>

#include <vkpp/framebuffer.hh>
#include <vkpp/image.hh>
#include <vkpp/sampler.hh>

#include <cstdint>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class DepthView final {
        public:
            DepthView(const std::uint32_t width, Rasterizer& vulkan_renderer,
                      const LightSource& light_source);
            DepthView(const std::uint32_t width,   const std::uint32_t height,
                      Rasterizer& vulkan_renderer, const LightSource& light_source);
            DepthView(const std::uint32_t width,   const std::uint32_t height,
                      Rasterizer& vulkan_renderer, const LightSource* light_source = nullptr);

            DepthView() = default;

            DepthView(Rasterizer& vulkan_renderer);

            void update_dynamic_viewport_scissor_depth(vk::CommandBuffer& cb);

            static VkFormat          get_attachment_format();
            static VkImageLayout     get_read_depth_layout();
            static VkImageLayout     get_attachment_layout();
            static VkImageUsageFlags get_image_usage_flags();

            vk::Sampler& get_sampler();
            vk::Framebuffer& get_framebuffer();
            vk::ImageView& get_image_view();

            const LightSource* light { nullptr };

        private:
            vk::Image image;
            vk::DeviceMemory memory;
            vk::ImageView image_view;
            vk::Framebuffer framebuffer;
            vk::Sampler sampler;

            VkViewport viewport;
            VkRect2D scissor;

            float constant_factor;
            float clamp;
            float slope_factor;

            static int id;
        };
    }
}

#endif