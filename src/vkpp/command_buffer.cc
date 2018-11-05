#include <vkpp/command_buffer.hh>

#include <vkpp/device.hh>
#include <vkpp/queue.hh>

#include <vkpp/exception.hh>

#include <algorithm>

#include <utility>

namespace vkpp {
    CommandBuffer::CommandBuffer(VkCommandBuffer& command_buffer,
                                 VkCommandPool&   command_pool,
                                 VkDevice& device, Queue* queue)
                                : queue_family { queue }, device { device },
                                  pool { command_pool }, handle { command_buffer } { }

    CommandBuffer::~CommandBuffer() noexcept {
        if (handle != VK_NULL_HANDLE) {
            queue_family->wait_idle();
            vkFreeCommandBuffers(device, pool, 1, &handle);
        }
    }

    CommandBuffer::CommandBuffer(CommandBuffer&& command_buffer) noexcept {
        swap(*this, command_buffer);
    }

    CommandBuffer& CommandBuffer::operator=(CommandBuffer&& command_buffer) noexcept {
        swap(*this, command_buffer);
        return *this;
    }

    void swap(CommandBuffer& lhs, CommandBuffer& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.pool, rhs.pool);
        swap(lhs.device, rhs.device);
        swap(lhs.queue_family, rhs.queue_family);

        rhs.queue_family = nullptr;

        rhs.handle = VK_NULL_HANDLE;
        rhs.pool = VK_NULL_HANDLE;
        rhs.device = VK_NULL_HANDLE;
    }

    VkCommandBuffer& CommandBuffer::get_handle() {
        return handle;
    }

    Queue& CommandBuffer::get_queue() {
        return *queue_family;
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags usage) {
        VkCommandBufferBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.pNext = nullptr;
        begin_info.flags = usage;

        begin_info.pInheritanceInfo = nullptr;

        if (VkResult error = vkBeginCommandBuffer(handle, &begin_info)) {
            throw Exception { error, "failed to start recording command buffer!" };
        }
    }
    void CommandBuffer::copy_buffer(Buffer& source, Buffer& destination,
                                    std::uint32_t source_offset,
                                    std::uint32_t destination_offset) {
        VkBufferCopy buffer_copy;

        buffer_copy.srcOffset = source_offset;
        buffer_copy.dstOffset = destination_offset;

        buffer_copy.size = std::min(source.get_size(), destination.get_size());

        vkCmdCopyBuffer(handle,
                        source.get_handle(), destination.get_handle(),
                        1, &buffer_copy);
    }

    void CommandBuffer::begin_render_pass(RenderPass& render_pass,
                                          Framebuffer& framebuffer,
                                          VkClearValue clear_color) {
        VkRenderPassBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.pNext = nullptr;

        begin_info.renderPass = render_pass.get_handle();
        begin_info.framebuffer = framebuffer.get_handle();
        begin_info.renderArea.extent = framebuffer.get_extent();
        begin_info.renderArea.offset = { 0, 0 };

        begin_info.pClearValues = &clear_color;
        begin_info.clearValueCount = 1;

        vkCmdBeginRenderPass(handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::bind_pipeline(Pipeline& pipeline) {
        vkCmdBindPipeline(handle, pipeline.get_bind_point(), pipeline.get_handle());
    }

    void CommandBuffer::bind_vertex_buffer(std::uint32_t first_binding,
                                           std::uint32_t binding_count,
                                           Buffer& vertex_buffer,
                                           VkDeviceSize vertex_offset) {
        vkCmdBindVertexBuffers(handle, first_binding, binding_count,
                               &vertex_buffer.get_handle(),
                               &vertex_offset);
    }

    void CommandBuffer::draw(std::uint32_t index_count, std::uint32_t instance_count,
                             std::uint32_t first_vertex, std::uint32_t first_instance) {
        vkCmdDraw(handle, index_count, instance_count, first_vertex, first_instance);
    }

    void CommandBuffer::end_render_pass() {
        vkCmdEndRenderPass(handle);
    }

    void CommandBuffer::end() {
        if (VkResult error = vkEndCommandBuffer(handle)) {
            throw Exception { error, "failed to record command buffer!" };
        }
    }

    CommandPool::CommandPool(Device& logical_device, Queue& queue)
                            : queue_family { &queue },
                              device { logical_device.get_handle() } {
        VkCommandPoolCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.queueFamilyIndex = queue.get_family_index();

        if (VkResult error = vkCreateCommandPool(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create command pool!" };
        }
    }

    CommandPool::~CommandPool() noexcept {
        if (handle != VK_NULL_HANDLE) {
            queue_family->wait_idle();
            vkDestroyCommandPool(device, handle, nullptr);
        }
    }

    CommandPool::CommandPool(CommandPool&& command_pool) noexcept {
        swap(*this, command_pool);
    }

    CommandPool& CommandPool::operator=(CommandPool&& command_pool) noexcept {
        swap(*this, command_pool);
        return *this;
    }

    void swap(CommandPool& lhs, CommandPool& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.queue_family, rhs.queue_family);
        swap(lhs.device, rhs.device);

        rhs.handle = VK_NULL_HANDLE;
        rhs.queue_family = nullptr;
        rhs.device = VK_NULL_HANDLE;
    }

    VkCommandPool& CommandPool::get_handle() {
        return handle;
    }

    Queue& CommandPool::get_queue() {
        return *queue_family;
    }

    void CommandPool::reset() {
        vkResetCommandPool(device, handle, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    CommandBuffer CommandPool::allocate(VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.commandPool = handle;
        alloc_info.commandBufferCount = 1;
        alloc_info.level = level;

        VkCommandBuffer cmd_handle;

        if (VkResult error = vkAllocateCommandBuffers(device, &alloc_info, &cmd_handle)) {
            throw Exception { error, "couldn't allocate command buffer!" };
        }

        return CommandBuffer { cmd_handle, handle, device, queue_family };
    }

    std::vector<CommandBuffer> CommandPool::allocate(std::uint32_t amount,
                                                     VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.commandPool = handle;
        alloc_info.commandBufferCount = amount;
        alloc_info.level = level;

        std::vector<VkCommandBuffer> cmds(amount);

        if (VkResult error = vkAllocateCommandBuffers(device, &alloc_info, cmds.data())) {
            throw Exception { error, "couldn't allocate command buffer!" };
        }

        std::vector<CommandBuffer> command_buffers;

        command_buffers.reserve(amount);

        for (auto command_buffer : cmds)
            command_buffers.emplace_back(command_buffer, handle,
                                         device, queue_family);

        return command_buffers;
    }
}
