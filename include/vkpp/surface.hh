#ifndef VKPP_SURFACE_HH
#define VKPP_SURFACE_HH

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Surface final {
    public:
        Surface() = default;
        Surface(VkInstance& instance,
                VkSurfaceKHR& surface);
        ~Surface() noexcept;

        Surface(Surface&& device) noexcept;
        Surface& operator=(Surface&& device) noexcept;

        friend void swap(Surface& lhs, Surface& rhs);

        void set_capabilities(const VkSurfaceCapabilitiesKHR& capabilities);
        void set_presentation_modes(const std::vector<VkPresentModeKHR>& present_modes);
        void set_formats(const std::vector<VkSurfaceFormatKHR>& formats);

        const VkSurfaceCapabilitiesKHR& get_capabilities() const;
        const std::vector<VkPresentModeKHR>& get_presentation_modes() const;
        const std::vector<VkSurfaceFormatKHR>& get_formats() const;

        VkSurfaceKHR& get_handle();

    private:
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkPresentModeKHR> present_modes;
        std::vector<VkSurfaceFormatKHR> formats;

        VkInstance instance { VK_NULL_HANDLE };
        VkSurfaceKHR handle { VK_NULL_HANDLE };
    };
}

#endif
