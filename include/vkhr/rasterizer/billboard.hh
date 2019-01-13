#ifndef VKHR_VULKAN_BILLBOARD_HH
#define VKHR_VULKAN_BILLBOARD_HH

#include <vkhr/scene_graph/billboard.hh>

#include <vkhr/rasterizer/pipeline.hh>
#include <vkhr/rasterizer/drawable.hh>

#include <vkhr/scene_graph/camera.hh>

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
                      vkhr::Rasterizer& vulkan_renderer);

            Billboard() = default;

            Billboard(const std::uint32_t width, const std::uint32_t height,
                      vkhr::Rasterizer& vulkan_renderer, bool flip = false);

            void load(const vkhr::Billboard& hair_style,
                      vkhr::Rasterizer& vulkan_rendrer);

            void send_img(vk::DescriptorSet&, Image&, vk::CommandBuffer&);

            void draw(Pipeline& pipeline,
                      vk::DescriptorSet& descriptor_set,
                      vk::CommandBuffer& command_buffer) override;

            static void build_pipeline(Pipeline& pipeline_reference,
                                       Rasterizer& vulkan_renderer);

        private:
            vk::ImageView billboard_view;
            vk::DeviceImage billboard_image;
            vk::Sampler billboard_sampler;

            static int id;
        };
    }
}

#endif