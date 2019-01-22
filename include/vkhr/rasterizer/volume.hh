#ifndef VKHR_VULKAN_VOLUME_HH
#define VKHR_VULKAN_VOLUME_HH

#include <vkhr/rasterizer/pipeline.hh>
#include <vkhr/rasterizer/drawable.hh>

#include <vkhr/scene_graph/camera.hh>

#include <vkpp/buffer.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/pipeline.hh>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class Volume : public Drawable {
        public:
            Volume() = default;

            void draw(Pipeline& vulkan_volume_rasterizer_pipeline,
                      vk::DescriptorSet& descriptor_set,
                      vk::CommandBuffer& command_buffer) override;

            static void build_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);

        private:
            static int id;
        };
    }
}

#endif