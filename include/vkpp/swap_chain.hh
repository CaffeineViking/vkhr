#ifndef VKPP_SWAP_CHAIN_HH
#define VKPP_SWAP_CHAIN_HH

#include <vkpp/device.hh>
#include <vkpp/image_view.hh>
#include <vkpp/surface.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class SwapChain final {
    public:
        enum class PresentationMode {
            Immediate = VK_PRESENT_MODE_IMMEDIATE_KHR,
            Fifo = VK_PRESENT_MODE_FIFO_KHR,
            FifoRelaxed = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
            MailBox = VK_PRESENT_MODE_MAILBOX_KHR
        };

        SwapChain() = default;
        SwapChain(Device& device, Surface& window_surface,
                  const VkSurfaceFormatKHR& preferred_format,
                  const PresentationMode& preferred_present_mode,
                  const VkExtent2D& preferred_window_extent);
        ~SwapChain() noexcept;

        SwapChain(SwapChain&& device) noexcept;
        SwapChain& operator=(SwapChain&& device) noexcept;

        friend void swap(SwapChain& lhs, SwapChain& rhs);

        static constexpr const char* DeviceExtension {
            "VK_KHR_swapchain"
        };

        VkSwapchainKHR& get_handle();

        Surface& get_surface() const;

        const std::vector<ImageView>& get_image_views() const;

        const VkExtent2D& get_current_extent() const;
        const PresentationMode& get_presentation_mode() const;
        const VkSurfaceFormatKHR& get_format() const;

    private:
        void create_swapchain_images();

        bool choose_format(const VkSurfaceFormatKHR& preferred_format);
        bool choose_mode(const PresentationMode& preferred_presentation_mode);

        void choose_extent(const VkExtent2D& window_extent);

        std::vector<VkImage>    images;
        std::vector<ImageView>  image_views;

        VkSurfaceFormatKHR format;
        PresentationMode presentation_mode;
        VkExtent2D current_extent;

        Surface* surface { nullptr };

        VkDevice device       { VK_NULL_HANDLE };
        VkSwapchainKHR handle { VK_NULL_HANDLE };
    };
}

#endif
