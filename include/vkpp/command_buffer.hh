#ifndef VKPP_COMMAND_BUFFER_HH
#define VKPP_COMMAND_BUFFER_HH

#include <vkpp/buffer.hh>
#include <vkpp/pipeline.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/framebuffer.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Device;
    class Queue;

    class CommandBuffer final {
    public:
        CommandBuffer() = default;

        CommandBuffer(VkCommandBuffer& command_buffer,
                      VkCommandPool& command_pool,
                      VkDevice& device, Queue* queue);

        ~CommandBuffer() noexcept;

        CommandBuffer(CommandBuffer&& command_buffer) noexcept;
        CommandBuffer& operator=(CommandBuffer&& command_buffer) noexcept;

        friend void swap(CommandBuffer& lhs, CommandBuffer& rhs);

        VkCommandBuffer& get_handle();

        Queue& get_queue();

        static constexpr auto SingleSubmit = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        static constexpr auto Simultaneous = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        void begin(VkCommandBufferUsageFlags = Simultaneous);

        void pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                              VkPipelineStageFlags destination_stage_mask,
                              VkMemoryBarrier memory_barrier);

        void pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                              VkPipelineStageFlags destination_stage_mask,
                              VkBufferMemoryBarrier buffer_memory_barrier);

        void pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                              VkPipelineStageFlags destination_stage_mask,
                              VkImageMemoryBarrier image_memory_barrier);

        void copy_buffer(Buffer& source, Buffer& destination,
                         std::uint32_t source_offset = 0,
                         std::uint32_t destination_offset = 0);

        void copy_buffer_image(Buffer& source, Image& destination);

        void begin_render_pass(RenderPass& render_pass,
                               Framebuffer& framebuffer,
                               VkClearValue clear_color);

        void bind_pipeline(Pipeline& pipeline);

        void bind_descriptor_set(DescriptorSet& descriptor_set,
                                 Pipeline& pipeline);

        void bind_vertex_buffer(std::uint32_t first_binding,
                                std::uint32_t binding_count,
                                Buffer& vertex_buffer,
                                VkDeviceSize vertex_offset = 0);
        void bind_vertex_buffer(VertexBuffer& vertex_buffer,
                                VkDeviceSize vertex_offset = 0);

        void draw(std::uint32_t index_count,
                  std::uint32_t instance_count = 1,
                  std::uint32_t first_vertex   = 0,
                  std::uint32_t first_instance = 0);

        void bind_index_buffer(Buffer& index_buffer,
                               VkIndexType type,
                               VkDeviceSize offset = 0);
        void bind_index_buffer(IndexBuffer& index_buffer,
                               VkDeviceSize offset = 0);

        void draw_indexed(std::uint32_t index_count,
                          std::uint32_t instance_count = 1,
                          std::uint32_t first_index = 0,
                          std::int32_t  vertex_offset  = 0,
                          std::uint32_t first_instance = 0);

        void end_render_pass();

        void end();

    private:
        Queue* queue_family    { nullptr };

        VkDevice        device { VK_NULL_HANDLE };
        VkCommandPool   pool   { VK_NULL_HANDLE };
        VkCommandBuffer handle { VK_NULL_HANDLE };
    };

    class CommandPool final {
    public:
        CommandPool() = default;

        CommandPool(Device& device, Queue& queue);

        ~CommandPool() noexcept;

        CommandPool(CommandPool&& command_pool) noexcept;
        CommandPool& operator=(CommandPool&& command_pool) noexcept;

        friend void swap(CommandPool& lhs, CommandPool& rhs);

        VkCommandPool& get_handle();

        Queue& get_queue();

        void reset();

        CommandBuffer allocate(VkCommandBufferLevel = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        static constexpr auto SingleSubmit = CommandBuffer::SingleSubmit;
        static constexpr auto Simultaneous = CommandBuffer::Simultaneous;

        CommandBuffer allocate_and_begin(VkCommandBufferUsageFlags = SingleSubmit);

        std::vector<CommandBuffer>
        allocate(std::uint32_t amount,
                 VkCommandBufferLevel command_level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    private:
        Queue* queue_family  { nullptr };
        VkDevice device      { VK_NULL_HANDLE };
        VkCommandPool handle { VK_NULL_HANDLE };
    };
}

#endif
