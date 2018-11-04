#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Device::Device(PhysicalDevice& physical_device,
                   const std::vector<Layer>& enabled_instance_layers,
                   const std::vector<Extension>& required_extensions,
                   const VkPhysicalDeviceFeatures& required_features)
                  : enabled_extensions { required_extensions },
                    enabled_features { required_features },
                    physical_device { &physical_device } {
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        float queue_priority = 1.0 / physical_device.get_queue_family_indices().size();

        for (const auto& queue_family_index : physical_device.get_queue_family_indices()) {
            VkDeviceQueueCreateInfo queue_create_info;
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.pNext = nullptr;
            queue_create_info.flags = 0;

            queue_create_info.queueFamilyIndex = queue_family_index;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queue_priority;

            queue_create_infos.push_back(queue_create_info);
        }

        auto missing_extensions = find(required_extensions);

        if (missing_extensions.size() != 0) {
            auto missing = collapse(missing_extensions);
            throw Exception { "couldn't create device!",
            "extension(s): " + missing + "are missing!"};
        }

        VkDeviceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        std::vector<const char*> extension_names(required_extensions.size());
        for (std::size_t i { 0 }; i < extension_names.size(); ++i)
            extension_names[i] = required_extensions[i].name.c_str();
        auto extension_count = (std::uint32_t) extension_names.size();

        // Device layers are actually deprecated, but we'll handle the
        // ones from the instance creation. We assume they're there :)

        std::vector<const char*> layers(enabled_instance_layers.size());
        for (std::size_t i { 0 }; i < layers.size(); ++i)
            layers[i] = enabled_instance_layers[i].name.c_str();
        auto layer_count = (std::uint32_t) layers.size();

        create_info.queueCreateInfoCount = queue_create_infos.size();
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.enabledLayerCount = layer_count;
        create_info.ppEnabledLayerNames = layers.data();
        create_info.enabledExtensionCount = extension_count;
        create_info.ppEnabledExtensionNames = extension_names.data();
        // TODO: for now assume the features exist, they might not...
        create_info.pEnabledFeatures = &required_features;

        if (VkResult error = vkCreateDevice(physical_device.get_handle(),
                                            &create_info, nullptr, &handle))
            throw Exception { error, "couldn't create device!" };

        assign_queues(); // Get the queue object from the device.
    }

    Device::~Device() noexcept {
        wait_idle(); // Until complete.
        if (handle != VK_NULL_HANDLE) {
            vkDestroyDevice(handle, nullptr);
        }
    }

    Device::Device(Device&& device) noexcept {
        swap(*this, device);
    }

    Device& Device::operator=(Device&& device) noexcept {
        swap(*this, device);
        return *this;
    }

    void swap(Device& lhs, Device& rhs) {
        using std::swap;

        swap(lhs.enabled_extensions, rhs.enabled_extensions);
        swap(lhs.enabled_features, rhs.enabled_features);

        swap(lhs.queues, rhs.queues);

        swap(lhs.graphics_queue, rhs.graphics_queue);
        swap(lhs.compute_queue, rhs.compute_queue);
        swap(lhs.transfer_queue, rhs.transfer_queue);
        swap(lhs.present_queue, rhs.present_queue);

        swap(lhs.physical_device, rhs.physical_device);

        swap(lhs.handle, rhs.handle);

        rhs.physical_device = nullptr;
        rhs.handle = VK_NULL_HANDLE;
    }

    VkDevice& Device::get_handle() {
        return handle;
    }

    PhysicalDevice& Device::get_physical_device() const {
        return *physical_device;
    }

    std::vector<Extension> Device::find(const std::vector<Extension>& extensions) const {
        return find(extensions, get_available_extensions());
    }

    const std::vector<Extension>& Device::get_enabled_extensions() const {
        return enabled_extensions;
    }

    const VkPhysicalDeviceFeatures& Device::get_available_features() const {
        return physical_device->get_features();
    }

    const std::vector<Extension>& Device::get_available_extensions() const {
        return physical_device->get_available_extensions();
    }

    bool Device::has_compute_queue() const {
        return compute_queue != nullptr;
    }

    bool Device::has_graphics_queue() const {
        return graphics_queue != nullptr;
    }

    bool Device::has_transfer_queue() const {
        return transfer_queue != nullptr;
    }

    bool Device::has_present_queue() const {
        return present_queue != nullptr;
    }

    void Device::wait_idle() {
        vkDeviceWaitIdle(handle);
    }

    Queue* Device::get_compute_queue() {
        return compute_queue;
    }

    Queue* Device::get_graphics_queue() {
        return graphics_queue;
    }

    Queue* Device::get_transfer_queue() {
        return transfer_queue;
    }

    Queue* Device::get_present_queue() {
        return present_queue;
    }

    void Device::assign_queues() {
        auto& physical_device = get_physical_device(); // Create Queues from the index.
        assign_queue(physical_device.get_compute_queue_family_index(), &compute_queue);
        assign_queue(physical_device.get_graphics_queue_family_index(), &graphics_queue);
        assign_queue(physical_device.get_transfer_queue_family_index(), &transfer_queue);
        assign_queue(physical_device.get_present_queue_family_index(), &present_queue);
    }

    void Device::assign_queue(std::int32_t index, Queue** queue) {
        if (queues.count(index) == 0) {
            VkQueue queue_handle;
            vkGetDeviceQueue(handle, index, 0, &queue_handle);
            queues[index] = Queue { queue_handle };
        }

        *queue = &queues[index];
    }
}
