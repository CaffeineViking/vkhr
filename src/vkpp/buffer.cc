#include <vkpp/buffer.hh>

#include <vkpp/queue.hh>
#include <vkpp/exception.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/device.hh>

#include <utility>

namespace vkpp {
    Buffer::Buffer(Device& logical_device,
                   VkDeviceSize size_in_bytes,
                   VkBufferUsageFlags usage)
                  : size { size_in_bytes },
                    sharing_mode { VK_SHARING_MODE_EXCLUSIVE },
                    usage { usage },
                    device { logical_device.get_handle() } {
        VkBufferCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.size = size_in_bytes;

        create_info.usage = usage;

        create_info.sharingMode = sharing_mode;

        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;

        if (VkResult error = vkCreateBuffer(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create buffer!" };
        }
    }

    Buffer::Buffer(Device& logical_device,
                   VkDeviceSize size_in_bytes,
                   VkBufferUsageFlags usage,
                   std::vector<Queue>& queue_families)
                  : size { size_in_bytes },
                    sharing_mode { VK_SHARING_MODE_CONCURRENT },
                    usage { usage },
                    device { logical_device.get_handle() } {
        VkBufferCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.size = size_in_bytes;

        create_info.sharingMode = sharing_mode;

        create_info.usage = usage;

        create_info.queueFamilyIndexCount = queue_families.size();

        std::vector<std::uint32_t> indices(queue_families.size());

        for (std::size_t i { 0 }; i < indices.size(); ++i) {
            indices[i] = queue_families[i].get_family_index();
        }

        create_info.pQueueFamilyIndices = indices.data();

        if (VkResult error = vkCreateBuffer(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create buffer!" };
        }
    }

    Buffer::~Buffer() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, handle, nullptr);
        }
    }

    Buffer::Buffer(Buffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    Buffer& Buffer::operator=(Buffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    void swap(Buffer& lhs, Buffer& rhs) {
        using std::swap;

        swap(lhs.device, rhs.device);
        swap(lhs.handle, rhs.handle);

        swap(lhs.size, rhs.size);
        swap(lhs.sharing_mode, rhs.sharing_mode);
        swap(lhs.usage, rhs.usage);
    }

    VkBuffer& Buffer::get_handle() {
        return handle;
    }

    VkMemoryRequirements Buffer::get_memory_requirements() const {
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    VkDeviceSize Buffer::get_size() const {
        return size;
    }

    VkSharingMode Buffer::get_sharing_mode() const {
        return sharing_mode;
    }

    VkBufferUsageFlags Buffer::get_usage() const {
        return usage;
    }

    VkDeviceMemory& Buffer::get_bound_memory() {
        return memory;
    }

    void Buffer::bind(DeviceMemory& device_memory, std::uint32_t offset) {
        memory = device_memory.get_handle();
        vkBindBufferMemory(device, handle, memory, offset);
    }

    void swap(VertexBuffer& lhs, VertexBuffer& rhs) {
        using std::swap;

        swap(static_cast<Buffer&>(lhs), static_cast<Buffer&>(rhs));

        swap(lhs.element_count, rhs.element_count);
        swap(lhs.device_memory, rhs.device_memory);
        swap(lhs.binding,       rhs.binding);
        swap(lhs.attributes,    rhs.attributes);
    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    void VertexBuffer::copy(Buffer& staged_buffer, CommandPool& pool) {
        auto command_buffer = pool.allocate();

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        command_buffer.copy_buffer(staged_buffer, *this);
        command_buffer.end();

        pool.get_queue().submit(command_buffer).wait_idle();
    }

    const VertexBuffer::Attributes& VertexBuffer::get_attributes() const {
        return attributes;
    }

    std::uint32_t VertexBuffer::get_binding_id() const {
        return binding.binding;
    }

    const VkVertexInputBindingDescription& VertexBuffer::get_binding() const {
        return binding;
    }

    DeviceMemory& VertexBuffer::get_device_memory() {
        return device_memory;
    }

    VkDeviceSize VertexBuffer::elements() const {
        return element_count;
    }

    void swap(UniformBuffer& lhs, UniformBuffer& rhs) {
        using std::swap;

        swap(static_cast<Buffer&>(lhs), static_cast<Buffer&>(rhs));

        swap(lhs.device_memory, rhs.device_memory);
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }
}
