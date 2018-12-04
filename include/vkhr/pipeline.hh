#ifndef VKHR_PIPELINE_HH
#define VKHR_PIPELINE_HH

#include <vkpp/pipeline.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/debug_marker.hh>
#include <vkpp/shader_module.hh>

#include <vector>

namespace vk = vkpp;

namespace vkhr {
    struct Pipeline {
        vk::GraphicsPipeline::FixedFunction fixed_stages;
        std::vector<vk::ShaderModule> shader_stages;
        vk::Pipeline::Layout pipeline_layout;
        vk::GraphicsPipeline pipeline;
        vk::DescriptorSet::Layout descriptor_set_layout;
        std::vector<vk::DescriptorSet> descriptor_sets;

        operator vk::Pipeline&() {
            return pipeline;
        }

        void make_current_pipeline(vk::CommandBuffer& command_list, std::size_t frame) {
            command_list.bind_pipeline(pipeline);
            command_list.bind_descriptor_set(descriptor_sets[frame], pipeline);
        }
    };
}

#endif
