#ifndef VKHR_VULKAN_PIPELINE_HH
#define VKHR_VULKAN_PIPELINE_HH

#include <vkpp/pipeline.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/debug_marker.hh>
#include <vkpp/shader_module.hh>

#include <vector>

namespace vk = vkpp;

namespace vkhr {
    namespace vulkan {
        struct Pipeline {
            vk::GraphicsPipeline::FixedFunction fixed_stages;
            vk::Pipeline::Layout pipeline_layout;
            vk::GraphicsPipeline pipeline;
            vk::DescriptorSet::Layout descriptor_set_layout;
            std::vector<vk::ShaderModule> shader_stages;

            struct DescriptorState {
                std::vector<vk::UniformBuffer> uniform_buffers;
                std::vector<vk::StorageBuffer> storage_buffers;
                std::vector<vk::CombinedImageSampler> samplers;
            };

            std::vector<vk::DescriptorSet> descriptor_sets;
            std::vector<DescriptorState> descriptor_states;

            void write_uniform_buffer(unsigned set, unsigned binding, VkDeviceSize uniform_buffers_bytes, vk::Device& device, const char* name)
            {
                std::string uniform_buffer_name { name };
                uniform_buffer_name += " Uniform Buffer";

                descriptor_states[set].uniform_buffers.emplace_back(device, uniform_buffers_bytes);
                descriptor_sets[set].write(binding, descriptor_states[set].uniform_buffers.back());

                vk::DebugMarker::object_name(device, descriptor_states[set].uniform_buffers.back(),
                                             VK_OBJECT_TYPE_BUFFER, uniform_buffer_name.c_str());
                std::string uniform_buffer_memory_name { name };
                uniform_buffer_memory_name += " Uniform Device Memory";
                vk::DebugMarker::object_name(device, descriptor_states[set].uniform_buffers.back().get_device_memory(),
                                             VK_OBJECT_TYPE_DEVICE_MEMORY, uniform_buffer_memory_name.c_str());
            }
        };
    }
}

#endif