#ifndef VKHR_VULKAN_HAIR_STYLE_HH
#define VKHR_VULKAN_HAIR_STYLE_HH

#include <vkhr/hair_style.hh>
#include <vkhr/vulkan/pipeline.hh>
#include <vkhr/paths.hh>

#include <vkpp/buffer.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/pipeline.hh>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class HairStyle final {
        public:
            HairStyle() = default;
            HairStyle(const vkhr::HairStyle& hair_style,
                      vkhr::Rasterizer& vulkan_renderer);

            void load(const vkhr::HairStyle& hair_style,
                      vkhr::Rasterizer& vulkan_renderer);

            void draw(vkpp::CommandBuffer& command_list,
                      vkpp::DescriptorSet& descriptors,
                      vkpp::GraphicsPipeline& pipeline);

            static Pipeline build_pipeline(Rasterizer&);

        private:
            vk::IndexBuffer  vertices;
            vk::VertexBuffer positions;
            vk::VertexBuffer tangents;
        };
    }
}

#endif