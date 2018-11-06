#ifndef VKPP_SWAP_CHAIN_HH
#define VKPP_SWAP_CHAIN_HH

#include <vkpp/image_view.hh>
#include <vkpp/semaphore.hh>
#include <vkpp/framebuffer.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/surface.hh>
#include <vkpp/fence.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Device;
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

        VkSwapchainKHR& get_handle();

        Surface& get_surface() const;

        std::uint32_t acquire_next_image(Fence& fence);
        std::uint32_t acquire_next_image(Semaphore& semaphore);

        std::vector<Framebuffer> create_framebuffers(RenderPass& render_pass);

        std::vector<ImageView>& get_image_views();
        VkImageView get_attachment(std::size_t i);

        std::uint32_t get_width()  const;
        std::uint32_t get_height() const;

        VkImageLayout get_layout() const;
        const VkExtent2D& get_extent() const;
        VkSampleCountFlagBits get_sample_count();
        const PresentationMode& get_presentation_mode() const;
        const VkColorSpaceKHR& get_color_space() const;
        const VkFormat& get_format() const;

        const VkSurfaceFormatKHR& get_surface_format() const;

        std::uint32_t size() const;

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
