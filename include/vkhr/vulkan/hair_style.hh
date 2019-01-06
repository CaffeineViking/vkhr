#ifndef VKHR_VULKAN_HAIR_STYLE_HH
#define VKHR_VULKAN_HAIR_STYLE_HH

#include <vkhr/hair_style.hh>
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
        class HairStyle final : public Drawable {
        public:
            HairStyle(const vkhr::HairStyle& hair_style,
                      vkhr::Rasterizer& vulkan_renderer);

            HairStyle() = default;

            void load(const vkhr::HairStyle& hair_style,
                      vkhr::Rasterizer& scene_renderer);

            void voxelize(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);

            void draw(Pipeline& vulkan_strand_rasterizer_pipeline,
                      vk::DescriptorSet& descriptor_set,
                      vk::CommandBuffer& command_buffer) override;

            void reduce_depth_buffer(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);
            void clip_curves(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);
            void prefix_sum_1(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);
            void prefix_sum_2(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);
            void reorder(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);
            void draw_strands(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer);

            static void build_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);
            static void depth_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);
            static void voxel_pipeline(Pipeline& pipeline_reference, Rasterizer& vulkan_renderer);

            enum ComputeCurve {
                ReduceDepthBuffer,
                ClipCurves,
                PrefixSum_1, PrefixSum_2,
                Reorder,
                DrawStrands
            };

            static void compute_curve_pipelines(std::unordered_map<ComputeCurve, Pipeline>& pipelines, Rasterizer&);

        private:
            vk::IndexBuffer  segments;
            vk::VertexBuffer vertices;
            vk::VertexBuffer tangents;

            struct Parameters {
                AABB volume_bounds;
                glm::vec3 volume_resolution;
                float strand_radius;
                glm::vec3 hair_color;
                float hair_shininess;
            } parameters;

            vk::UniformBuffer parameters_buffer;

            vk::ImageView density_view;
            vk::DeviceImage density_volume;
            vk::Sampler density_sampler;

            static int id;
        };
    }
}

#endif