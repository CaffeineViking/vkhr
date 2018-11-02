#ifndef VKPP_SURFACE_HH
#define VKPP_SURFACE_HH

#include <vkpp/physical_device.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class PhysicalDevice;
    class Surface final {
    public:
        Surface(VkInstance& instance,
                VkSurfaceKHR& surface);
        ~Surface() noexcept;

        operator VkSurfaceKHR() const;

        VkSurfaceKHR& get_handle();

        PhysicalDevice* get_active_physical_device() const;
        void set_active_physical_device(PhysicalDevice& physical_device);

        enum class PresentMode {
            Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
            Mailbox = VK_PRESENT_MODE_MAILBOX_KHR,
            Fifo = VK_PRESENT_MODE_FIFO_KHR,
            FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR
        };

        const VkSurfaceCapabilitiesKHR& get_capabilities() const;
        const std::vector<VkSurfaceFormatKHR>& get_formats() const;
        const std::vector<PresentMode>& get_present_modes() const;

    private:
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<PresentMode> present_modes;

        PhysicalDevice* physical_device { nullptr };

        VkInstance instance { VK_NULL_HANDLE };
        VkSurfaceKHR handle { VK_NULL_HANDLE };
    };
}

#endif
