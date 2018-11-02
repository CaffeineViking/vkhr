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

        vkEnumerateDeviceExtensionProperties(handle, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extension_properties(count);
        vkEnumerateDeviceExtensionProperties(handle, nullptr, &count,
                                             extension_properties.data());

        available_extensions.resize(extension_properties.size());
        for (std::size_t i { 0 }; i < available_extensions.size(); ++i)
            available_extensions[i] = extension_properties[i];

        locate_queue_family_indices(); // Find queue families.
        assign_queue_family_indices(); // Added to queue list.
    }

    void PhysicalDevice::locate_queue_family_indices() {
        for (std::size_t i { 0 }; i < queue_families.size(); ++i) {
            if (queue_families[i].queueCount > 0) {
                if (!has_graphics_queue() &&
                    queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    graphics_queue_family_index = i;
                if (!has_compute_queue() &&
                    queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
                    compute_queue_family_index  = i;
                if (!has_transfer_queue() &&
                    queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
                    transfer_queue_family_index = i;
            }
        }
    }

    void PhysicalDevice::assign_queue_family_indices() {
        queue_family_indices.clear();
        if (graphics_queue_family_index != -1)
            queue_family_indices.insert(graphics_queue_family_index);
        if (compute_queue_family_index != -1)
            queue_family_indices.insert(compute_queue_family_index);
        if (transfer_queue_family_index != -1)
            queue_family_indices.insert(transfer_queue_family_index);
        if (present_queue_family_index != -1)
            queue_family_indices.insert(present_queue_family_index);
    }

    void PhysicalDevice::find_device_memory_heap_index() {
        for (std::size_t i { 0 }; i < memory_properties.memoryHeapCount; ++i) {
            if (memory_properties.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                device_heap_index = i;
                return;
            }
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

    std::uint32_t PhysicalDevice::get_device_memory_heap() const {
        return device_heap_index;
    }

    VkDeviceSize PhysicalDevice::get_device_memory_size() const {
        auto heap = get_device_memory_heap();
        return memory_properties.memoryHeaps[heap].size;
    }

    const VkPhysicalDeviceFeatures& PhysicalDevice::get_features() const {
        return features;
    }

    const VkPhysicalDeviceMemoryProperties& PhysicalDevice::get_memory_properties() const {
        return memory_properties;
    }

    const std::vector<VkQueueFamilyProperties>&
    PhysicalDevice::get_queue_family_properties() const {
        return queue_families;
    }

    const std::vector<Extension>& PhysicalDevice::get_available_extensions() const {
        return available_extensions;
    }

    const VkPhysicalDeviceProperties& PhysicalDevice::get_properties() const {
        return properties;
    }

    void PhysicalDevice::assign_present_queue_indices(Surface& surface) {
        VkBool32 presentation_supported { 0 };
        for (std::size_t i { 0 }; i < queue_families.size(); ++i) {
            vkGetPhysicalDeviceSurfaceSupportKHR(handle, i, surface.get_handle(),
                                                 &presentation_supported);
            if (presentation_supported) {
                present_queue_family_index = i;
                break;
            }
        }

        // Presentation, format, capability.
        query_surface_capabilities(surface);

        assign_queue_family_indices();
    }

    bool PhysicalDevice::has_present_queue(Surface& surface) const {
        if (!has_present_queue()) {
            VkBool32 presentation_supported { 0 };
            for (std::size_t i { 0 }; i < queue_families.size(); ++i) {
                vkGetPhysicalDeviceSurfaceSupportKHR(handle, i, surface.get_handle(),
                                                     &presentation_supported);
                if (presentation_supported) return true;
            }
        } else {
            return true;
        }

        return false;
    }

    const std::unordered_set<std::int32_t>&
    PhysicalDevice::get_queue_family_indices() const {
        return queue_family_indices;
    }

    bool PhysicalDevice::has_every_queue() const {
        return has_compute_queue()  &&
               has_graphics_queue() &&
               has_transfer_queue();
    }

    bool PhysicalDevice::has_compute_queue() const {
        return compute_queue_family_index != -1;
    }

    bool PhysicalDevice::has_graphics_queue() const {
        return graphics_queue_family_index != -1;
    }

    bool PhysicalDevice::has_transfer_queue() const {
        return transfer_queue_family_index != -1;
    }

    bool PhysicalDevice::has_present_queue() const {
        return present_queue_family_index != -1;
    }

    std::int32_t PhysicalDevice::get_compute_queue_family_index() const {
        return compute_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_graphics_queue_family_index() const {
        return graphics_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_transfer_queue_family_index() const {
        return transfer_queue_family_index;
    }

    std::int32_t PhysicalDevice::get_present_queue_family_index() const {
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


    void PhysicalDevice::query_surface_capabilities(Surface& window_surface) {
        auto& surface = window_surface.get_handle();

        VkSurfaceCapabilitiesKHR capabilities;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &capabilities);

        window_surface.set_capabilities(capabilities);

        std::uint32_t count;

        std::vector<VkSurfaceFormatKHR> formats;

        vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, nullptr);
        formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &count, formats.data());

        window_surface.set_formats(formats);

        std::vector<VkPresentModeKHR> present_modes;

        vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count, nullptr);
        present_modes.resize(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &count,
                                                  present_modes.data());

        window_surface.set_presentation_modes(present_modes);
    }
}
