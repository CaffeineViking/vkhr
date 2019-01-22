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
            Volume(vkhr::Rasterizer& vk_renderer);

            Volume() = default;

            void load(vkhr::Rasterizer& renderer);

            void set_current_volume(vk::ImageView& volume_view);
            void set_parameter_buffer(vk::UniformBuffer& param);
            void set_volume_sampler(vk::Sampler& voxel_sampler);

            std::vector<glm::vec3> generate_cube_vertices() const;
            std::vector<unsigned>  generate_cube_elements() const;

            void draw(Pipeline& vulkan_volume_rasterizer_pipeline,
                      vk::DescriptorSet& descriptor_set,
                      vk::CommandBuffer& command_buffer) override;

            static void build_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);

        private:
            vk::IndexBuffer  elements;
            vk::VertexBuffer vertices;

            vk::ImageView* volume_view  { nullptr };
            vk::UniformBuffer* parameter_buffer { nullptr };
            vk::Sampler* volume_sampler { nullptr };

            static int id;
        };
    }
}

#endif