#ifndef VKPP_BUFFER_HH
#define VKPP_BUFFER_HH

#include <vkpp/device_memory.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Queue;
    class Device;

    class Buffer final {
    public:
        Buffer() = default;

        Buffer(Device& device,
               VkDeviceSize size_in_bytes,
               VkBufferUsageFlags usage);

        Buffer(Device& device,
               VkDeviceSize size,
               VkBufferUsageFlags usage,
               std::vector<Queue>& queue_families);

        ~Buffer() noexcept;

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

    private:
        VkDeviceSize size;
        VkSharingMode sharing_mode;
        VkBufferUsageFlags usage;

        VkDeviceMemory  memory { VK_NULL_HANDLE };

        VkDevice device { VK_NULL_HANDLE };
        VkBuffer handle { VK_NULL_HANDLE };
    };
}

#endif
