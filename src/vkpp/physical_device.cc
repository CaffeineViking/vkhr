#include <vkpp/physical_device.hh>

namespace vkpp {
    PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& physical_device)
                                  : handle { physical_device } {
        vkGetPhysicalDeviceProperties(handle, &properties);
        vkGetPhysicalDeviceMemoryProperties(handle, &memory_properties);
        vkGetPhysicalDeviceFeatures(handle, &features);

        find_device_memory_heap_index();

        name = properties.deviceName;
        type = static_cast<Type>(properties.deviceType);

        std::uint32_t count;
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, nullptr);
        queue_families.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(handle, &count,
                                                 queue_families.data());

        locate_queue_family_indices(); // Queues.
    }

    void PhysicalDevice::locate_queue_family_indices() {
        for (std::size_t i { 0 }; i < queue_families.size(); ++i) {
            if (queue_families[i].queueCount > 0) {
                if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    graphics_queue_family_index = i;
                if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                    compute_queue_family_index  = i;
                if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                    transfer_queue_family_index = i;
            }
        }
    }

    void PhysicalDevice::find_device_memory_heap_index() {
        for (std::size_t i { 0 }; i < memory_properties.memoryHeapCount; ++i) {
            if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                device_heap_index = i;
        }
    }

    PhysicalDevice::operator VkPhysicalDevice() const {
        return handle;
    }

    bool PhysicalDevice::operator==(const PhysicalDevice& other) const {
        return handle == other.handle;
    }

    bool PhysicalDevice::operator!=(const PhysicalDevice& other) const {
        return !(*this == other);
    }

    VkPhysicalDevice& PhysicalDevice::get_handle() {
        return handle;
    }

    bool PhysicalDevice::is_gpu() const {
        return is_discrete_gpu() ||
               is_integrated_gpu() ||
               is_virtual_gpu();
    }

    bool PhysicalDevice::is_discrete_gpu() const {
        return type == Type::DiscreteGpu;
    }

    bool PhysicalDevice::is_integrated_gpu() const {
        return type == Type::IntegratedGpu;
    }

    bool PhysicalDevice::is_virtual_gpu() const {
        return type == Type::VirtualGpu;
    }

    std::uint32_t PhysicalDevice::get_memory_heap() const {
        return device_heap_index;
    }

    VkDeviceSize PhysicalDevice::get_memory_size() const {
        auto heap = get_memory_heap();
        return memory_properties.memoryHeaps[heap].size;
    }

    const VkPhysicalDeviceFeatures& PhysicalDevice::get_features() const {
        return features;
    }

    const VkPhysicalDeviceMemoryProperties& PhysicalDevice::get_memory_properties() const {
        return memory_properties;
    }

    const std::vector<VkQueueFamilyProperties> PhysicalDevice::get_queue_families() const {
        return queue_families;
    }

    const VkPhysicalDeviceProperties& PhysicalDevice::get_properties() const {
        return properties;
    }

    std::int32_t PhysicalDevice::get_compute_queue() const {
        return compute_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_graphics_queue() const {
        return graphics_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_transfer_queue() const {
        return transfer_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_present_queue() const {
        return present_queue_family_index;
    }

    std::string PhysicalDevice::get_details() const {
        return get_name() + " (" + get_type_string() + ")";
    }

    const std::string& PhysicalDevice::get_name() const {
        return name;
    }

    PhysicalDevice::Type PhysicalDevice::get_type() const {
        return type;
    }

    std::string PhysicalDevice::get_type_string() const {
        switch (type) {
        case Type::Other: return "Other";
        case Type::IntegratedGpu: return "Integrated GPU";
        case Type::DiscreteGpu: return "Discrete GPU";
        case Type::VirtualGpu: return "Virtual GPU";
        case Type::Cpu: return "Host CPU";
        default: return "?";
        }
    }
}
