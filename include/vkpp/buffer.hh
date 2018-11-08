#ifndef VKPP_BUFFER_HH
#define VKPP_BUFFER_HH

#include <vkpp/device_memory.hh>

#include <vulkan/vulkan.h>

#include <vector>
#include <set>

namespace vkpp {
    class Queue;
    class CommandPool;
    class Device;

    class Buffer {
    public:
        Buffer() = default;

        Buffer(Device& device,
               VkDeviceSize size_in_bytes,
               VkBufferUsageFlags usage);

        Buffer(Device& device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               std::vector<Queue>& queue_families);

        virtual ~Buffer() noexcept;

        Buffer(Buffer&& buffer) noexcept;
        Buffer& operator=(Buffer&& buffer) noexcept;

        friend void swap(Buffer& lhs, Buffer& rhs);

        VkBuffer& get_handle();

        VkDeviceSize get_size() const;
        VkDeviceMemory& get_bound_memory();
        VkMemoryRequirements get_memory_requirements() const;

        VkSharingMode get_sharing_mode() const;
        VkBufferUsageFlags get_usage() const;

        void bind(DeviceMemory& device_memory, std::uint32_t offset = 0);

    protected:
        VkDeviceSize size;
        VkSharingMode sharing_mode;
        VkBufferUsageFlags usage;

        VkDeviceMemory  memory { VK_NULL_HANDLE };

        VkDevice device { VK_NULL_HANDLE };
        VkBuffer handle { VK_NULL_HANDLE };
    };

    class DeviceBuffer : public Buffer {
    public:
        DeviceBuffer() = default;

        friend void swap(DeviceBuffer& lhs, DeviceBuffer& rhs);
        DeviceBuffer& operator=(DeviceBuffer&& buffer) noexcept;
        DeviceBuffer(DeviceBuffer&& buffer) noexcept;

        template<typename T>
        DeviceBuffer(Device& device,
                     CommandPool& pool,
                     std::vector<T>& data,
                     VkBufferUsageFlags usage);

        DeviceMemory& get_device_memory();

    protected:
        void copy(Buffer& staged, CommandPool& pool);

        DeviceMemory device_memory;
    };

    class HostBuffer : public Buffer {
    public:
        HostBuffer() = default;

        friend void swap(HostBuffer& lhs, HostBuffer& rhs);
        HostBuffer& operator=(HostBuffer&& buffer) noexcept;
        HostBuffer(HostBuffer&& buffer) noexcept;

        template<typename T>
        HostBuffer(Device& device,
                   std::vector<T>& data,
                   VkBufferUsageFlags usage);

        template<typename T>
        HostBuffer(Device& device,
                   T& data_object,
                   VkBufferUsageFlags usage);

        DeviceMemory& get_device_memory();

    protected:
        DeviceMemory device_memory;
    };

    class VertexBuffer : public DeviceBuffer {
    public:
        struct Attribute {
            std::uint32_t location;
            VkFormat format;
            std::uint32_t offset { 0 };
        };

        using Bindings   = std::vector<VkVertexInputBindingDescription>;
        using Attributes = std::vector<VkVertexInputAttributeDescription>;

        VertexBuffer() = default;

        friend void swap(VertexBuffer& lhs, VertexBuffer& rhs);
        VertexBuffer& operator=(VertexBuffer&& buffer) noexcept;
        VertexBuffer(VertexBuffer&& buffer) noexcept;

        template<typename T>
        VertexBuffer(Device& device,
                     CommandPool& pool,
                     std::vector<T>& vertices,
                     std::uint32_t binding,
                     const std::vector<Attribute> attributes);

        std::uint32_t get_binding_id() const;

        const VkVertexInputBindingDescription& get_binding() const;

        const Attributes& get_attributes() const;

        VkDeviceSize elements() const;

    private:
        VkDeviceSize element_count;
        VkVertexInputBindingDescription binding;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    class IndexBuffer : public DeviceBuffer {
    public:
        IndexBuffer() = default;

        friend void swap(IndexBuffer& lhs, IndexBuffer& rhs);
        IndexBuffer& operator=(IndexBuffer&& buffer) noexcept;
        IndexBuffer(IndexBuffer&& buffer) noexcept;

        template<typename T>
        IndexBuffer(Device& device,
                     CommandPool& pool,
                     std::vector<T>& indices);

        VkDeviceSize elements() const;

    private:
        VkDeviceSize element_count;
    };

    class UniformBuffer : public HostBuffer {
    public:
        UniformBuffer() = default;

        friend void swap(UniformBuffer& lhs, UniformBuffer& rhs);
        UniformBuffer& operator=(UniformBuffer&& buffer) noexcept;
        UniformBuffer(UniformBuffer&& buffer) noexcept;

        template<typename T>
        UniformBuffer(Device& device,
                      T& data_object);

        template<typename T>
        void update(T& update_object);
    };

    template<typename T>
    DeviceBuffer::DeviceBuffer(Device& device,
                               CommandPool& pool,
                               std::vector<T>& buffer,
                               VkBufferUsageFlags usage)
                              : Buffer { device,
                                         sizeof(buffer[0]) * buffer.size(),
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage } {
        Buffer staging_buffer {
            device,
            sizeof(buffer[0]) * buffer.size(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };

        auto staging_memory_requirements = staging_buffer.get_memory_requirements();

        DeviceMemory staging_memory {
            device,
            staging_memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        staging_buffer.bind(staging_memory);
        staging_memory.copy(buffer, 0);

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

    template<typename T>
    HostBuffer::HostBuffer(Device& device,
                           std::vector<T>& buffer,
                           VkBufferUsageFlags usage)
                          : Buffer { device,
                                     sizeof(buffer[0]) * buffer.size(),
                                     usage } {
        auto memory_requirements = get_memory_requirements();

        device_memory = DeviceMemory {
            device,
            memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        bind(device_memory);
        device_memory.copy(buffer, 0);
    }

    template<typename T>
    HostBuffer::HostBuffer(Device& device,
                           T& data_object,
                           VkBufferUsageFlags usage)
                          : Buffer { device,
                                     sizeof(data_object),
                                     usage } {
        auto memory_requirements = get_memory_requirements();
        device_memory = DeviceMemory {
            device,
            memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        bind(device_memory);
        device_memory.copy(data_object, 0);
    }

    template<typename T>
    VertexBuffer::VertexBuffer(Device& device,
                               CommandPool& pool,
                               std::vector<T>& vertices,
                               std::uint32_t binding,
                               const std::vector<Attribute> attributes)
                              : DeviceBuffer { device,
                                               pool,
                                               vertices,
                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT } {
        this->attributes.reserve(attributes.size());
        for (const auto& attribute : attributes) {
            this->attributes.push_back({ attribute.location,
                                         binding,
                                         attribute.format,
                                         attribute.offset });
        }

        this->element_count = vertices.size();

        this->binding  = { binding, sizeof(vertices[0]), VK_VERTEX_INPUT_RATE_VERTEX };
    }

    template<typename T>
    IndexBuffer::IndexBuffer(Device& device,
                             CommandPool& pool,
                             std::vector<T>& indices)
                            : DeviceBuffer { device,
                                               pool,
                                               indices,
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT } {
        this->element_count = indices.size();
    }

    template<typename T>
    UniformBuffer::UniformBuffer(Device& device, T& data_object)
                                : HostBuffer { device,  data_object,
                                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT } { }

    template<typename T>
    void UniformBuffer::update(T& update_object) {
        device_memory.copy(update_object);
    }
}

#endif
