#ifndef VKHR_VULKAN_MODEL_HH
#define VKHR_VULKAN_MODEL_HH

#include <vkhr/model.hh>
#include <vkhr/pipeline.hh>
#include <vkhr/vulkan/drawable.hh>
#include <vkhr/camera.hh>

#include <vkpp/buffer.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/pipeline.hh>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class Model final : public Drawable {
        public:
            Model(const vkhr::Model& wavefront_model,
                  vkhr::Rasterizer& vulkan_renderer);

            Model() = default;

            void load(const vkhr::Model& wavefront_model,
                      vkhr::Rasterizer& vulkan_renderer);

            void draw(vk::CommandBuffer& command_buffer) override;

            static void build_pipeline(Pipeline& pipeline_reference,
                                       Rasterizer& vulkan_renderer);

        private:
            static int id;
        };
    }
}

#endif