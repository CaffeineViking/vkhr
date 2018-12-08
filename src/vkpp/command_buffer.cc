#include <vkpp/command_buffer.hh>

#include <vkhr/vulkan/depth_view.hh>

#include <vkpp/device.hh>
#include <vkpp/debug_marker.hh>
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

    void CommandBuffer::pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                                         VkPipelineStageFlags destination_stage_mask,
                                         VkMemoryBarrier memory_barrier) {
        vkCmdPipelineBarrier(handle, source_stage_mask, destination_stage_mask, 0,
                             1, &memory_barrier,
                             0, nullptr,
                             0, nullptr);
    }

    void CommandBuffer::pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                                         VkPipelineStageFlags destination_stage_mask,
                                         VkBufferMemoryBarrier buffer_memory_barrier) {
        vkCmdPipelineBarrier(handle, source_stage_mask, destination_stage_mask, 0,
                             0, nullptr,
                             1, &buffer_memory_barrier,
                             0, nullptr);
    }

    void CommandBuffer::pipeline_barrier(VkPipelineStageFlags source_stage_mask,
                                         VkPipelineStageFlags destination_stage_mask,
                                         VkImageMemoryBarrier image_memory_barrier) {
        vkCmdPipelineBarrier(handle, source_stage_mask, destination_stage_mask, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &image_memory_barrier);
    }

    void CommandBuffer::blit_image(Image& source, Image& destination, VkFilter filter) {
        VkOffset3D blit_size;
        blit_size.x = destination.get_extent().width;
        blit_size.y = destination.get_extent().height;
        blit_size.z = 1;

        VkImageBlit blit_region {  };

        blit_region.srcSubresource.aspectMask = source.get_aspect_mask();
        blit_region.srcSubresource.layerCount = 1;
        blit_region.srcOffsets[1] = blit_size;

        blit_region.dstSubresource.aspectMask = destination.get_aspect_mask();
        blit_region.dstSubresource.layerCount = 1;
        blit_region.dstOffsets[1] = blit_size;

        vkCmdBlitImage(handle, source.get_handle(), source.get_layout(),
                       destination.get_handle(), destination.get_layout(),
                       1, &blit_region, filter);
    }

    void CommandBuffer::copy_image(Image& source, Image& destination) {
        VkImageCopy copy_region {  };

        copy_region.srcSubresource.aspectMask = source.get_aspect_mask();
        copy_region.srcSubresource.layerCount = 1;
        copy_region.dstSubresource.aspectMask = destination.get_aspect_mask();
        copy_region.dstSubresource.layerCount = 1;

        copy_region.extent = destination.get_extent();

        vkCmdCopyImage(handle, source.get_handle(), source.get_layout(),
                       destination.get_handle(), destination.get_layout(),
                       1, &copy_region);
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

    void CommandBuffer::copy_buffer_image(Buffer& source, Image& destination) {
        VkBufferImageCopy region;

        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = destination.get_extent();

        vkCmdCopyBufferToImage(handle,
                               source.get_handle(), destination.get_handle(),
                               destination.get_layout(),
                               1, &region);
    }

    void CommandBuffer::begin_render_pass(RenderPass& render_pass,
                                          vkhr::vulkan::DepthView& depth_map) {
        VkRenderPassBeginInfo begin_info;
        begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        begin_info.pNext = nullptr;

        begin_info.renderPass = render_pass.get_handle();
        begin_info.framebuffer = depth_map.get_framebuffer().get_handle();
        begin_info.renderArea.extent = depth_map.get_framebuffer().get_extent();
        begin_info.renderArea.offset = { 0, 0 };

        VkClearValue depth_clear_value { };
        depth_clear_value.depthStencil = { 1.000f, 0u };

        begin_info.pClearValues    = &depth_clear_value;
        begin_info.clearValueCount = 1;

        vkCmdBeginRenderPass(handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
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

        std::vector<VkClearValue> clear_values { clear_color };

        if (render_pass.has_depth_attachment()) {
            VkClearValue depth_clear_value {  };
            depth_clear_value.depthStencil = { 1, 0 };
            clear_values.push_back(depth_clear_value);
        }

        begin_info.pClearValues    = clear_values.data();
        begin_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());

        vkCmdBeginRenderPass(handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void CommandBuffer::set_viewport(VkViewport& viewport) {
        vkCmdSetViewport(handle, 0, 1, &viewport);
    }

    void CommandBuffer::set_scissor(VkRect2D& new_scissor) {
        vkCmdSetScissor(handle, 0, 1, &new_scissor);
    }

    void CommandBuffer::set_depth_bias(float constant_factor,
                                       float clamp,
                                       float slope_factor) {
        vkCmdSetDepthBias(handle, constant_factor,
                                  clamp,
                                  slope_factor);
    }

    void CommandBuffer::bind_pipeline(Pipeline& pipeline) {
        vkCmdBindPipeline(handle, pipeline.get_bind_point(), pipeline.get_handle());
    }

    void CommandBuffer::bind_descriptor_set(DescriptorSet& descriptor_set,
                                            Pipeline& pipeline) {
        vkCmdBindDescriptorSets(handle, pipeline.get_bind_point(),
                                pipeline.get_layout().get_handle(),
                                0, 1, &descriptor_set.get_handle(),
                                0, nullptr);
    }

    void CommandBuffer::bind_vertex_buffer(std::uint32_t first_binding,
                                           std::uint32_t binding_count,
                                           Buffer& vertex_buffer,
                                           VkDeviceSize vertex_offset) {
        vkCmdBindVertexBuffers(handle, first_binding, binding_count,
                               &vertex_buffer.get_handle(),
                               &vertex_offset);
    }

    void CommandBuffer::bind_vertex_buffer(VertexBuffer& vertex_buffer,
                                           VkDeviceSize vertex_offset) {
        bind_vertex_buffer(vertex_buffer.get_binding_id(), vertex_buffer, vertex_offset);
    }

    void CommandBuffer::bind_vertex_buffer(std::uint32_t binding_number,
                                           VertexBuffer& vertex_buffer,
                                           VkDeviceSize vertex_offset) {
        bind_vertex_buffer(binding_number, 1, vertex_buffer, vertex_offset);
    }

    void CommandBuffer::draw(std::uint32_t index_count, std::uint32_t instance_count,
                             std::uint32_t first_vertex, std::uint32_t first_instance) {
        vkCmdDraw(handle, index_count, instance_count, first_vertex, first_instance);
    }

    void CommandBuffer::bind_index_buffer(Buffer& index_buffer,
                                          VkIndexType type,
                                          VkDeviceSize offset) {
        vkCmdBindIndexBuffer(handle, index_buffer.get_handle(), offset, type);
    }

    void CommandBuffer::bind_index_buffer(IndexBuffer& index_buffer,
                                          VkDeviceSize offset) {
        vkCmdBindIndexBuffer(handle, index_buffer.get_handle(),
                             offset, index_buffer.get_type());
    }

    void CommandBuffer::draw_indexed(std::uint32_t index_count,
                                     std::uint32_t instance_count,
                                     std::uint32_t first_index,
                                     std::int32_t  vertex_offset,
                                     std::uint32_t first_instance) {
        vkCmdDrawIndexed(handle, index_count, instance_count,
                         first_index, vertex_offset, first_instance);
    }

    void CommandBuffer::end_render_pass() {
        vkCmdEndRenderPass(handle);
    }

    void CommandBuffer::end() {
        if (VkResult error = vkEndCommandBuffer(handle)) {
            throw Exception { error, "failed to record command buffer!" };
        }
    }

    void CommandBuffer::reset_query_pool(QueryPool& query_pool, std::uint32_t first_query, std::uint32_t query_count) {
        vkCmdResetQueryPool(handle, query_pool.get_handle(), first_query, query_count);
        query_pool.query = 0;
    }

    void CommandBuffer::begin_query(QueryPool& query_pool, std::uint32_t index, VkQueryControlFlags flags) {
        vkCmdBeginQuery(handle, query_pool.get_handle(), index, flags);
    }

    void CommandBuffer::copy_query_pool_results(QueryPool& query_pool,
                                                std::uint32_t first_query, std::uint32_t query_count,
                                                Buffer& destination, VkQueryResultFlags result_flags,
                                                VkDeviceSize offset, VkDeviceSize stride) {
        vkCmdCopyQueryPoolResults(handle, query_pool.get_handle(),
                                  first_query, query_count,
                                  destination.get_handle(), offset, stride,
                                  result_flags);
    }

    void CommandBuffer::write_timestamp(QueryPool& query_pool, VkPipelineStageFlagBits pipeline_stage, std::uint32_t query) {
        vkCmdWriteTimestamp(handle, pipeline_stage, query_pool.get_handle(), query);
    }

    void CommandBuffer::end_query(QueryPool& query_pool, std::uint32_t index) {
        vkCmdEndQuery(handle, query_pool.get_handle(), index);
    }

    CommandPool::CommandPool(Device& logical_device, Queue& queue)
                            : queue_family { &queue },
                              device { logical_device.get_handle() } {
        VkCommandPoolCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        create_info.queueFamilyIndex = queue.get_family_index();

        if (VkResult error = vkCreateCommandPool(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create command pool!" };
        }

        DebugMarker::object_name(device, *this, VK_OBJECT_TYPE_COMMAND_POOL, "Command Pool");
    }

    CommandPool::~CommandPool() noexcept {
        if (handle != VK_NULL_HANDLE) {
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

    CommandBuffer CommandPool::allocate_and_begin(VkCommandBufferUsageFlags with_usage) {
        auto command_buffer = allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        command_buffer.begin(with_usage);
        return std::move(command_buffer);
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
