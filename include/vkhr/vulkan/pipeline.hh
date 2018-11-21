#ifndef VKHR_VULKAN_PIPELINE_HH
#define VKHR_VULKAN_PIPELINE_HH

#include <vkpp/pipeline.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/shader_module.hh>

namespace vk = vkpp;

namespace vkhr {
    namespace vulkan {
        struct Pipeline {
            vk::GraphicsPipeline::FixedFunction fixed_stages;
            vk::Pipeline::Layout pipeline_layout;
            vk::GraphicsPipeline pipeline;
            std::vector<vk::DescriptorSet> descriptors;
            vk::DescriptorSet::Layout descriptor_set_layout;
            std::vector<vk::ShaderModule> shader_stages;
        };
    }
}

#endif