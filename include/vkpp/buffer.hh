#ifndef VKPP_BUFFER_HH
#define VKPP_BUFFER_HH

#include <vkpp/device_memory.hh>

#include <vulkan/vulkan.h>

#include <vector>
#include <cstring>
#include <set>

namespace vkpp {
    class Queue;
    class CommandPool;
    class CommandBuffer;
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
               std::vector<Queue>& fam);

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

        void bind(DeviceMemory& device_memory,
                  std::uint32_t offset = 0);

    protected:
        VkDeviceSize size_in_bytes;
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

        DeviceBuffer(Device& device,
                     CommandPool& command_buffer,
                     const void* buffer,
                     VkDeviceSize size,
                     VkBufferUsageFlags usage);

        DeviceBuffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage);

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
                     CommandPool& command_buffer,
                     const std::vector<T>& vertices,
                     std::uint32_t binding = 0,
                     const std::vector<Attribute> attributes = {});

        std::uint32_t get_binding_id() const;

        const VkVertexInputBindingDescription& get_binding() const;

        const Attributes& get_attributes() const;

        std::uint32_t count() const;

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

        IndexBuffer(Device& device,
                    CommandPool& command_buffer,
                    const std::vector<unsigned short>& indices);

        IndexBuffer(Device& device,
                    CommandPool& command_buffer,
                    const std::vector<unsigned>& indices);

        VkIndexType get_type() const;

        std::uint32_t count() const;

    private:
        VkDeviceSize element_count;
        VkIndexType  index_type;
    };

    class StorageBuffer : public DeviceBuffer {
    public:
        StorageBuffer() = default;

        friend void swap(StorageBuffer& lhs, StorageBuffer& rhs);
        StorageBuffer& operator=(StorageBuffer&& buffer) noexcept;
        StorageBuffer(StorageBuffer&& buffer) noexcept;

        template<typename T>
        StorageBuffer(Device& device,
                      CommandPool& command_pool,
                      const T& buffer);

        template<typename T>
        StorageBuffer(Device& device,
                      CommandPool& command_pool,
                      const std::vector<T>& vector);

        StorageBuffer(Device& device, VkDeviceSize size);

        Buffer& get_staging_buffer();

        DeviceMemory& get_staging_memory();

        template<typename T> void staged_copy(T& object,              CommandBuffer& command_buffer);
        template<typename T> void staged_copy(T& object,              CommandPool& command_pool);
        template<typename T> void staged_copy(std::vector<T>& vector, CommandBuffer& command_buffer);
        template<typename T> void staged_copy(std::vector<T>& vector, CommandPool& command_pool);

    private:
        Buffer       staging_buffer;
        DeviceMemory staging_memory;
    };

    class HostBuffer : public Buffer {
    public:
        HostBuffer() = default;

        friend void swap(HostBuffer& lhs, HostBuffer& rhs);
        HostBuffer& operator=(HostBuffer&& buffer) noexcept;
        HostBuffer(HostBuffer&& buffer) noexcept;

        HostBuffer(Device& device,
                   const void* buffer,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage);

        HostBuffer(Device& device,
                   VkDeviceSize size,
                   VkBufferUsageFlags usage);

        DeviceMemory& get_device_memory();

    protected:
        DeviceMemory device_memory;
    };

    class UniformBuffer : public HostBuffer {
    public:
        UniformBuffer() = default;

        friend void swap(UniformBuffer& lhs, UniformBuffer& rhs);
        UniformBuffer& operator=(UniformBuffer&& buffer) noexcept;
        UniformBuffer(UniformBuffer&& buffer) noexcept;

        template<typename T>
        UniformBuffer(Device& device,
                      const T& buffer);

        template<typename T>
        UniformBuffer(Device& device,
                      const std::vector<T>& vector);

        UniformBuffer(Device& device,
                      VkDeviceSize size);

        static std::vector<UniformBuffer> create(Device& device, VkDeviceSize size, std::size_t n = 1, const char* name = "");

        template<typename T> void update(std::vector<T>& vec);
        template<typename T> void update(T& uniform_data_obj);
    };

    template<typename T>
    void StorageBuffer::staged_copy(T& object, CommandBuffer& command_buffer) {
        void* data = reinterpret_cast<void*>(&object);
        staging_memory.copy(sizeof(T), data);
        command_buffer.copy_buffer(staging_buffer, *this);
    }

    template<typename T>
    void StorageBuffer::staged_copy(T& object, CommandPool& command_pool) {
        auto command_buffer = command_pool.allocate_and_begin();
        command_buffer.copy_buffer(*this, staging_buffer);
        command_buffer.end();

        T* data { nullptr };
        staging_memory.map(0, get_size(), (void**) &data);
        std::memcpy(&object, data, get_size());
        staging_memory.unmap();
    }

    template<typename T>
    void StorageBuffer::staged_copy(std::vector<T>& vector, CommandPool& command_pool) {
        auto command_buffer = command_pool.allocate_and_begin();
        command_buffer.copy_buffer(*this, staging_buffer);
        command_buffer.end();

        command_pool.get_queue().submit(command_buffer).wait_idle();

        T* data { nullptr };
        vector.resize(get_size() / sizeof(T));
        staging_memory.map(0, get_size(), (void**) &data);
        std::memcpy(vector.data(), data, get_size());
        staging_memory.unmap();
    }

    template<typename T>
    void StorageBuffer::staged_copy(std::vector<T>& vector, CommandBuffer& command_buffer) {
        staging_memory.copy(sizeof(T) * vector.size(), vector.data());
        command_buffer.copy_buffer(staging_buffer, *this);
    }

    template<typename T>
    VertexBuffer::VertexBuffer(Device& device,
                               CommandPool& command_buffer,
                               const std::vector<T>& vertices,
                               std::uint32_t binding,
                               const std::vector<Attribute> attributes)
                              : DeviceBuffer { device,
                                               command_buffer,
                                               vertices.data(),
                                               sizeof(vertices[0]) * vertices.size(),
                                               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT } {
        this->attributes.reserve(attributes.size());
        for (const auto& attribute : attributes) {
            this->attributes.push_back({ attribute.location,
                                         binding,
                                         attribute.format,
                                         attribute.offset });
        }

        this->element_count = vertices.size();

        this->binding = { binding, sizeof(vertices[0]), VK_VERTEX_INPUT_RATE_VERTEX };
    }

    template<typename T>
    StorageBuffer::StorageBuffer(Device& device,
                                 CommandPool& command_pool,
                                 const T& buffer)
                                : DeviceBuffer { device, command_pool, &buffer, sizeof(T),
                                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT } {  }

    template<typename T>
    StorageBuffer::StorageBuffer(Device& device,
                                 CommandPool& command_pool,
                                 const std::vector<T>& vector)
                                : DeviceBuffer { device, command_pool, vector.data(),
                                                 vector.size() * sizeof(T),
                                                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT } {  }

    template<typename T>
    UniformBuffer::UniformBuffer(Device& device,
                                 const T& buffer)
                                : HostBuffer { device, &buffer, sizeof(T),
                                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT } { }

    template<typename T>
    UniformBuffer::UniformBuffer(Device& device,
                                 const std::vector<T>& vector)
                                : HostBuffer { device, vector.data(),
                                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT } { }

    template<typename T>
    void UniformBuffer::update(T& data_object) {
        void* data = reinterpret_cast<void*>(&data_object);
        device_memory.copy(device_memory.get_size(), data);
    }

    template<typename T>
    void UniformBuffer::update(std::vector<T>& data_vector) {
        device_memory.copy(data_vector.size() * sizeof(T),
                           data_vector.data());
    }
}

#endif
