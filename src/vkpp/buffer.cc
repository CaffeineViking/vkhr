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

    DeviceBuffer::DeviceBuffer(Device& device,
                               CommandPool& pool,
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
        copy(staging_buffer,
             pool);
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

    void DeviceBuffer::copy(Buffer& staged_buffer, CommandPool& pool) {
        auto command_buffer = pool.allocate_and_begin();
        command_buffer.copy_buffer(staged_buffer, *this);
        command_buffer.end();
        pool.get_queue().submit(command_buffer)
                        .wait_idle();
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

    VkDeviceSize VertexBuffer::count() const {
        return element_count;
    }
    template<> IndexBuffer::IndexBuffer<unsigned>(Device& device,
                                       CommandPool& pool,
                                       const std::vector<unsigned>& indices)
                                      : DeviceBuffer { device,
                                                       pool,
                                                       indices.data(),
                                                       sizeof(indices[0]) * indices.size(),
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT } {
        this->element_count = indices.size();
        this->index_type    = VK_INDEX_TYPE_UINT32;
    }

    template<> IndexBuffer::IndexBuffer<unsigned short>(Device& device,
                                       CommandPool& pool,
                                       const std::vector<unsigned short>& indices)
                                      : DeviceBuffer { device,
                                                       pool,
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
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& buffer) noexcept {
        swap(*this, buffer);
        return *this;
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& buffer) noexcept {
        swap(*this, buffer);
    }

    VkDeviceSize IndexBuffer::count() const {
        return element_count;
    }

    VkIndexType IndexBuffer::get_type() const {
        return index_type;
    }

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
