#ifndef VKPP_COMMAND_BUFFER_HH
#define VKPP_COMMAND_BUFFER_HH

#include <vkpp/buffer.hh>
#include <vkpp/pipeline.hh>
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

        CommandBuffer(VkCommandBuffer& command_buffer);

        VkCommandBuffer& get_handle();

        static constexpr auto Default = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        void begin(VkCommandBufferUsageFlags = Default);

        void begin_render_pass(RenderPass& render_pass,
                               Framebuffer& framebuffer,
                               VkClearValue clear_color);

        void bind_pipeline(Pipeline& pipeline);

        void draw(std::uint32_t index_count,
                  std::uint32_t instance_count,
                  std::uint32_t first_vertex   = 0,
                  std::uint32_t first_instance = 0);

        void bind_vertex_buffer(std::uint32_t first_binding,
                                std::uint32_t binding_count,
                                Buffer& vertex_buffer,
                                VkDeviceSize vertex_offset);

        void end_render_pass();

        void end();

    private:
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
