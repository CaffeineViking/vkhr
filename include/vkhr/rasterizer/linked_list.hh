#ifndef VKHR_VULKAN_LINKED_LIST_HH
#define VKHR_VULKAN_LINKED_LIST_HH

#include <vkhr/rasterizer/pipeline.hh>

#include <vkpp/buffer.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/image.hh>

namespace vk = vkpp;

namespace vkhr {
    class Rasterizer;
    namespace vulkan {
        class LinkedList {
        public:
            LinkedList(vkhr::Rasterizer& rasterizer,  std::uint32_t width, std::uint32_t height, std::size_t node_size, std::size_t node_count);

            LinkedList() = default;

            void create(vkhr::Rasterizer& rasterizer, std::uint32_t width, std::uint32_t height, std::size_t node_size, std::size_t node_count);

            void clear(vk::CommandBuffer& command_buffer);

            void resolve(vk::SwapChain& swap_chain, std::uint32_t frame, Pipeline& ppll_resolving_pipeline, vk::CommandBuffer& command_buffers);

            static constexpr std::size_t AverageFragmentsPerPixel = 16; // Only a estimated average fragments per pixel.
            static constexpr std::size_t NodeSize = 16; // { [R, G, B, A], Fragment Depth, Index To Previous Fragment }.
            static constexpr std::uint32_t Null = 0xffffffff; // Encodes end of some list (or an invalid entry somehow).

            std::size_t get_width() const;
            std::size_t get_node_count() const;
            std::size_t get_node_size() const;
            std::size_t get_height() const;

            std::size_t get_heads_size_in_bytes() const;
            std::size_t get_nodes_size_in_bytes() const;

            vk::DeviceImage& get_heads();
            vk::ImageView& get_heads_view();
            vk::StorageBuffer& get_node_counter();
            vk::UniformBuffer& get_parameters();
            vk::StorageBuffer& get_nodes();

            static void build_pipeline(Pipeline& pipeline, Rasterizer& rasterizer); // Builds the PPLL resolve pipeline.

        private:
            std::size_t width;
            std::size_t node_size;
            std::size_t height;

            struct Parameters {
                std::uint32_t node_count;
            } parameters_buffer;

            VkClearColorValue null_value;

            vk::DeviceImage   heads;
            vk::ImageView     heads_view;
            vk::StorageBuffer node_counter;
            vk::UniformBuffer parameters;
            vk::StorageBuffer nodes;

            static int id;
        };
    }
}

#endif