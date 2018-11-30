#include <vkpp/buffer.hh>

#include <vkpp/queue.hh>
#include <vkpp/exception.hh>
#include <vkpp/debug_marker.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/device.hh>

#include <utility>

namespace vkpp {
    Buffer::Buffer(Device& logical_device,
                   VkDeviceSize size_in_bytes,
                   VkBufferUsageFlags usage)
                  : size_in_bytes { size_in_bytes },
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
                  : size_in_bytes { size_in_bytes },
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

        swap(lhs.size_in_bytes, rhs.size_in_bytes);
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
        return size_in_bytes;
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

    DeviceBuffer::DeviceBuffer(Device& device,
                               CommandPool& command_pool,
                               const void* buffer,
                               VkDeviceSize size,
                               VkBufferUsageFlags usage)
                              : Buffer { device,
                                         size,
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage } {
        Buffer staging_buffer {
            device,
            size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };

        auto staging_memory_requirements = staging_buffer.get_memory_requirements();

        DeviceMemory staging_memory {
            device,
            staging_memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        staging_buffer.bind(staging_memory);
        staging_memory.copy(size, buffer);

        auto buffer_memory_requirements = get_memory_requirements();

        device_memory = DeviceMemory {
            device,
            buffer_memory_requirements,
            DeviceMemory::Type::DeviceLocal
        };

        bind(device_memory);

        auto command_buffer = command_pool.allocate_and_begin();
        command_buffer.copy_buffer(staging_buffer, *this);
        command_buffer.end();

        command_pool.get_queue().submit(command_buffer)
                                .wait_idle();
    }

    void swap(DeviceBuffer& lhs, DeviceBuffer& rhs) {
        using std::swap;

        swap(static_cast<Buffer&>(lhs), static_cast<Buffer&>(rhs));

        swap(lhs.device_memory, rhs.device_memory);
    }

    DeviceBuffer& DeviceBuffer::operator=(DeviceBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    DeviceBuffer::DeviceBuffer(DeviceBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    DeviceMemory& DeviceBuffer::get_device_memory() {
        return device_memory;
    }

    HostBuffer::HostBuffer(Device& device,
                           const void* buffer,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage)
                          : Buffer { device,
                                     size,
                                     usage } {
        auto memory_requirements = get_memory_requirements();

        device_memory = DeviceMemory {
            device,
            memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        bind(device_memory);
        device_memory.copy(size, buffer);
    }

    HostBuffer::HostBuffer(Device& device,
                           VkDeviceSize size,
                           VkBufferUsageFlags usage)
                          : Buffer { device,
                                     size,
                                     usage } {
        auto memory_requirements = get_memory_requirements();

        device_memory = DeviceMemory {
            device,
            memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        bind(device_memory);
    }

    void swap(HostBuffer& lhs, HostBuffer& rhs) {
        using std::swap;

        swap(static_cast<Buffer&>(lhs), static_cast<Buffer&>(rhs));

        swap(lhs.device_memory, rhs.device_memory);
    }

    HostBuffer& HostBuffer::operator=(HostBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    HostBuffer::HostBuffer(HostBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    DeviceMemory& HostBuffer::get_device_memory() {
        return device_memory;
    }

    void swap(VertexBuffer& lhs, VertexBuffer& rhs) {
        using std::swap;

        swap(static_cast<DeviceBuffer&>(lhs), static_cast<DeviceBuffer&>(rhs));

        swap(lhs.element_count, rhs.element_count);
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

    const VertexBuffer::Attributes& VertexBuffer::get_attributes() const {
        return attributes;
    }

    std::uint32_t VertexBuffer::get_binding_id() const {
        return binding.binding;
    }

    const VkVertexInputBindingDescription& VertexBuffer::get_binding() const {
        return binding;
    }

    std::uint32_t VertexBuffer::count() const {
        return static_cast<std::uint32_t>(element_count);
    }

    IndexBuffer::IndexBuffer(Device& device,
                             CommandPool& command_pool,
                             const std::vector<unsigned>& indices)
                            : DeviceBuffer { device,
                                             command_pool,
                                             indices.data(),
                                             sizeof(indices[0]) * indices.size(),
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT } {
        this->element_count = indices.size();
        this->index_type    = VK_INDEX_TYPE_UINT32;
    }

    IndexBuffer::IndexBuffer(Device& device,
                             CommandPool& command_pool,
                             const std::vector<unsigned short>& indices)
                            : DeviceBuffer { device,
                                             command_pool,
                                             indices.data(),
                                             sizeof(indices[0]) * indices.size(),
                                             VK_BUFFER_USAGE_INDEX_BUFFER_BIT } {
        this->element_count = indices.size();
        this->index_type    = VK_INDEX_TYPE_UINT16;
    }

    void swap(IndexBuffer& lhs, IndexBuffer& rhs) {
        using std::swap;

        swap(static_cast<DeviceBuffer&>(lhs), static_cast<DeviceBuffer&>(rhs));

        swap(lhs.element_count, rhs.element_count);
        swap(lhs.index_type, rhs.index_type);
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    std::uint32_t IndexBuffer::count() const {
        return static_cast<std::uint32_t>(element_count);
    }

    VkIndexType IndexBuffer::get_type() const {
        return index_type;
    }

    UniformBuffer::UniformBuffer(Device& device,
                                 VkDeviceSize size)
        : HostBuffer { device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT } {  }

    void swap(UniformBuffer& lhs, UniformBuffer& rhs) {
        using std::swap;
        swap(static_cast<HostBuffer&>(lhs), static_cast<HostBuffer&>(rhs));
    }

    UniformBuffer& UniformBuffer::operator=(UniformBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    UniformBuffer::UniformBuffer(UniformBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    std::vector<UniformBuffer> UniformBuffer::create(Device& device, VkDeviceSize size, std::size_t n, const char* name) {
        std::vector<UniformBuffer> uniform_buffers;
        uniform_buffers.reserve(n);
        for (std::size_t i { 0 }; i < n; ++i) {
            uniform_buffers.emplace_back(device, size);
            DebugMarker::object_name(device, uniform_buffers.back(),
                                     VK_OBJECT_TYPE_BUFFER, name);
        } return uniform_buffers;
    }

    void swap(StorageBuffer& lhs, StorageBuffer& rhs) {
        using std::swap;
        swap(static_cast<DeviceBuffer&>(lhs), static_cast<DeviceBuffer&>(rhs));
    }

    StorageBuffer& StorageBuffer::operator=(StorageBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    StorageBuffer::StorageBuffer(StorageBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }
}
