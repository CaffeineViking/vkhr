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

    class VertexBuffer : public Buffer {
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

        DeviceMemory& get_device_memory();

        std::uint32_t get_binding_id() const;

        const VkVertexInputBindingDescription& get_binding() const;

        const Attributes& get_attributes() const;

        VkDeviceSize elements() const;

    private:
        void copy(Buffer& staged, CommandPool& command_pool);

        VkDeviceSize element_count;
        DeviceMemory device_memory;
        VkVertexInputBindingDescription binding;
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    template<typename T>
    VertexBuffer::VertexBuffer(Device& device,
                               CommandPool& pool,
                               std::vector<T>& vertices,
                               std::uint32_t binding,
                               const std::vector<Attribute> attributes)
                              : Buffer { device,
                                         sizeof(vertices[0]) * vertices.size(),
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
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

        Buffer staging_buffer {
            device,
            sizeof(vertices[0]) * vertices.size(),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT
        };

        auto staging_memory_requirements = staging_buffer.get_memory_requirements();

        DeviceMemory staging_memory {
            device,
            staging_memory_requirements,
            DeviceMemory::Type::HostVisible
        };

        staging_buffer.bind(staging_memory);
        staging_memory.copy(vertices, 0);

        auto vertex_memory_requirements = get_memory_requirements();

        device_memory = DeviceMemory {
            device,
            vertex_memory_requirements,
            DeviceMemory::Type::DeviceLocal
        };

        bind(device_memory);
        copy(staging_buffer,
             pool);

    }
}

#endif
