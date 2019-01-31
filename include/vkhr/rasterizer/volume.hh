#ifndef VKHR_VULKAN_VOLUME_HH
#define VKHR_VULKAN_VOLUME_HH

#include <vkhr/rasterizer/pipeline.hh>
#include <vkhr/scene_graph/hair_style.hh>
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
        class HairStyle;
        class Volume : public Drawable {
        public:
            Volume(HairStyle& hair_style, vkhr::Rasterizer& vk_renderer);

            Volume() = default;

            void load(HairStyle& hair_style, vkhr::Rasterizer& renderer);

            void set_current_volume(vk::ImageView& density_view, vk::ImageView& tangent_view);
            void set_volume_parameters(vk::UniformBuffer& buffer);
            void set_volume_sampler(vk::Sampler& density_sample, vk::Sampler& tangent_sampler);

            std::vector<glm::vec3> generate_aabb_vertices(const AABB& aabb) const;
            std::vector<unsigned>  generate_aabb_elements() const;

            void draw(Pipeline& vulkan_volume_rasterizer_pipeline,
                      vk::DescriptorSet& descriptor_set,
                      vk::CommandBuffer& command_buffer) override;

            static void build_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);

        private:
            vk::IndexBuffer  elements;
            vk::VertexBuffer vertices;

            vk::ImageView* tangent_view  { nullptr };
            vk::ImageView* density_view  { nullptr };
            vk::UniformBuffer* parameter_buffer { nullptr };
            vk::Sampler* density_sampler { nullptr };
            vk::Sampler* tangent_sampler { nullptr };

            static int id;
        };
    }
}

#endif