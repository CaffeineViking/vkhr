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
        vk::ComputePipeline  compute_pipeline;
        vk::DescriptorSet::Layout descriptor_set_layout;
        std::vector<vk::DescriptorSet> descriptor_sets;

        operator vk::Pipeline&() {
            if (compute_pipeline.get_handle() != VK_NULL_HANDLE)
                return compute_pipeline;
            return pipeline;
        }
    };
}

#endif
