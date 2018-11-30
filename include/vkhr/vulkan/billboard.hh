#ifndef VKHR_VULKAN_BILLBOARD_HH
#define VKHR_VULKAN_BILLBOARD_HH

#include <vkhr/billboard.hh>
#include <vkhr/vulkan/pipeline.hh>
#include <vkhr/vulkan/drawable.hh>
#include <vkhr/camera.hh>
#include <vkhr/paths.hh>

#include <vkpp/buffer.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/pipeline.hh>
#include <vkpp/image.hh>
#include <vkpp/sampler.hh>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class Billboard final : public Drawable {
        public:
            Billboard(const vkhr::Billboard& billboards,
                      vkhr::Rasterizer& vulkan_renderer,
                      Pipeline& billboard_gpu_pipeline);

            Billboard() = default;

            Billboard(const std::uint32_t width, const std::uint32_t height,
                      vkhr::Rasterizer& vulkan_renderer, Pipeline& pipeline);

            void load(const vkhr::Billboard& hair_style,
                      vkhr::Rasterizer& vulkan_rendrer);
            void update(Image& i, vk::CommandBuffer&, std::size_t j);
            void update(vk::ImageView&, vk::Sampler&, std::size_t i);
            void draw(vk::CommandBuffer& command_buffer,
                      std::size_t framebuffer) override;

            Pipeline* pipeline { nullptr };

            static void build_pipeline(Pipeline& pipeline_reference,
                                       Rasterizer& vulkan_renderer);

        private:
            vk::ImageView billboard_view;
            vk::DeviceImage billboard_image;
            vk::Sampler billboard_sampler;
        };
    }
}

#endif