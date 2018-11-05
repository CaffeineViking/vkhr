#include <vkpp/device_memory.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

#include <cstring>

namespace vkpp {
    DeviceMemory::DeviceMemory(Device& logical_device,
                               VkDeviceSize size_in_bytes,
                               std::uint32_t type)
                              : type { type }, size { size_in_bytes },
                                device { logical_device.get_handle() } {
        VkMemoryAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.allocationSize = size_in_bytes;
        alloc_info.memoryTypeIndex = type;

        if (VkResult error = vkAllocateMemory(device, &alloc_info, nullptr, &handle)) {
            throw Exception { error, "couldn't allocate device memory!" };
        }
    }

    DeviceMemory::DeviceMemory(Device& logical_device, VkMemoryRequirements requirements)
                              : size { requirements.size },
                                device { logical_device.get_handle() } {
        VkMemoryAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.allocationSize = size;

        auto& physical_device = logical_device.get_physical_device();

        alloc_info.memoryTypeIndex = physical_device.find_memory(requirements);

        if (VkResult error = vkAllocateMemory(device, &alloc_info, nullptr, &handle)) {
            throw Exception { error, "couldn't allocate device memory!" };
        }
    }

    DeviceMemory::~DeviceMemory() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkFreeMemory(device, handle, nullptr);
        }
    }

    DeviceMemory::DeviceMemory(DeviceMemory&& memory) noexcept {
        swap(*this, memory);
    }

    DeviceMemory& DeviceMemory::operator=(DeviceMemory&& memory) noexcept {
        swap(*this, memory);
        return *this;
    }

    void swap(DeviceMemory& lhs, DeviceMemory& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);

        swap(lhs.size, rhs.size);
        swap(lhs.type, rhs.type);

        rhs.device = VK_NULL_HANDLE;
        rhs.handle = VK_NULL_HANDLE;
    }

    VkDeviceMemory& DeviceMemory::get_handle() {
        return handle;
    }

    VkDeviceSize DeviceMemory::get_size() const {
        return size;
    }

    std::uint32_t DeviceMemory::get_type() const {
        return type;
    }

    void DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size, void** data) {
        vkMapMemory(device, handle, offset, size, 0, data);
    }

    void DeviceMemory::unmap() {
        vkUnmapMemory(device, handle);
    }

    void DeviceMemory::copy(VkDeviceSize size, const void* data, VkDeviceSize offset) {
        void* mapped_memory;
        map(offset, size, &mapped_memory);
        std::memcpy(mapped_memory, data, static_cast<std::size_t>(size));
        unmap();
    }
}
