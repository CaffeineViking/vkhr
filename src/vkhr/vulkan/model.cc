#include <vkhr/vulkan/model.hh>

#include <vkhr/camera.hh>
#include <vkhr/light_source.hh>
#include <vkhr/rasterizer.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        Model::Model(const vkhr::Model& wavefront_model,
                     vkhr::Rasterizer& vulkan_renderer) {
        }

        void Model::load(const vkhr::Model& wavefront_model,
                         vkhr::Rasterizer& vulkan_renderer) {
        }

        void Model::draw(vk::CommandBuffer& command_buffer) {
        }

        void Model::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
        }

        int Model::id { 0 };
    }
}