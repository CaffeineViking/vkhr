#include <vkpp/surface.hh>

#include <vkpp/exception.hh>

namespace vkpp {
    Surface::Surface(VkInstance& instance,
                     VkSurfaceKHR& surface)
                    : instance { instance },
                      handle { surface } { }

    Surface::~Surface() noexcept {
        if (handle != VK_NULL_HANDLE && instance != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, handle, nullptr);
        }
    }

    Surface::operator VkSurfaceKHR() const {
        return handle;
    }

    VkSurfaceKHR& Surface::get_handle() {
        return handle;
    }

    PhysicalDevice* Surface::get_active_physical_device() const {
        return physical_device;
    }

    void Surface::set_active_physical_device(PhysicalDevice& physical_device) {
        this->physical_device = &physical_device;
        auto device_handle = physical_device.get_handle();

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_handle, handle, &capabilities);

        std::uint32_t count;

        vkGetPhysicalDeviceSurfaceFormatsKHR(device_handle, handle, &count, nullptr);
        formats.resize(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device_handle, handle, &count, formats.data());

        vkGetPhysicalDeviceSurfacePresentModesKHR(device_handle, handle, &count, nullptr);
        std::vector<VkPresentModeKHR> present_modes(count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device_handle, handle, &count,
                                                  present_modes.data());

        this->present_modes.resize(count);

        for (std::size_t i { 0 }; i < this->present_modes.size(); ++i) {
            this->present_modes[i] = static_cast<PresentMode>(present_modes[i]);
        }
    }

    const VkSurfaceCapabilitiesKHR& Surface::get_capabilities() const {
        if (physical_device != nullptr) {
            return capabilities;
        } else {
            throw Exception { "couldn't get surface capabilities!",
            "there are no physical devices bound to the surface" };
        }
    }

    const std::vector<VkSurfaceFormatKHR>& Surface::get_formats() const {
        if (physical_device != nullptr) {
            return formats;
        } else {
            throw Exception { "couldn't get the surface formats!",
            "there are no physical devices bound to the surface" };
        }
    }

    const std::vector<Surface::PresentMode>& Surface::get_present_modes() const {
        if (physical_device != nullptr) {
            return present_modes;
        } else {
            throw Exception { "couldn't get surface present modes!",
            "there are no physical devices bound to the surface" };
        }
    }
}
