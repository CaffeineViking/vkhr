#include <vkhr/rasterizer/volume.hh>

#include <vkhr/rasterizer.hh>

#include <vkhr/scene_graph/camera.hh>
#include <vkhr/scene_graph/light_source.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        void Volume::draw(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
        }

        void Volume::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
        }

        int Volume::id { 0 };
    }
}