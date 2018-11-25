#include <vkpp/surface.hh>

#include <vkpp/exception.hh>

namespace vkpp {
    Surface::Surface(VkInstance& instance,
                     VkSurfaceKHR& surface)
                    : instance { instance },
                      handle { surface } { }

    Surface::~Surface() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, handle, nullptr);
        }
    }

    Surface::Surface(Surface&& surface) noexcept {
        swap(*this, surface);
    }

    Surface& Surface::operator=(Surface&& surface) noexcept {
        swap(*this, surface);
        return *this;
    }

    void swap(Surface& lhs, Surface& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.instance, rhs.instance);
        swap(lhs.window, rhs.window);
    }

    VkSurfaceKHR& Surface::get_handle() {
        return handle;
    }

    void Surface::set_glfw_window(vkhr::Window& window) {
        this->window = &window;
    }

    vkhr::Window& Surface::get_glfw_window() const {
        return *this->window;
    }

    void Surface::set_capabilities(const VkSurfaceCapabilitiesKHR& capabilities) {
        this->capabilities = capabilities;
    }

    void Surface::set_presentation_modes(const std::vector<VkPresentModeKHR>& modes) {
        this->present_modes = modes;
    }

    void Surface::set_formats(const std::vector<VkSurfaceFormatKHR>& formats) {
        this->formats = formats;
    }

    const VkSurfaceCapabilitiesKHR& Surface::get_capabilities() const {
        return capabilities;
    }

    const std::vector<VkPresentModeKHR>& Surface::get_presentation_modes() const {
        return present_modes;
    }

    const std::vector<VkSurfaceFormatKHR>& Surface::get_formats() const {
        return formats;
    }
}
