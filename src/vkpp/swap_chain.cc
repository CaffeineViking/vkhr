#include <vkpp/swap_chain.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <algorithm>
#include <utility>
#include <limits>

namespace vkpp {
    SwapChain::SwapChain(Device& logical_device, Surface& surface,
                         const VkSurfaceFormatKHR& preferred_format,
                         const PresentationMode& preferred_present_mode,
                         const VkExtent2D& preferred_window_extent)
                        : surface { &surface },
                          device { logical_device.get_handle() } {
        if (surface.get_presentation_modes().size() == 0 ||
            surface.get_formats().size() == 0) {
            throw Exception { "couldn't create swap chain!",
            "no present mode or format found on surface" };
        }

        if (!choose_format(preferred_format)) {
            throw Exception { "couldn't create swap chain!",
            "no suitable surface format was found here!" };
        }

        if (!choose_mode(preferred_present_mode)) {
            throw Exception { "couldn't create swap chain!",
            "no suitable presentation modes were found!" };
        }

        choose_extent(preferred_window_extent);

        std::uint32_t image_count { surface.get_capabilities().minImageCount };

        if (surface.get_capabilities().maxImageCount > 0 &&
            image_count > surface.get_capabilities().maxImageCount) {
            image_count = surface.get_capabilities().maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info;

        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.surface = surface.get_handle();

        create_info.minImageCount = image_count;
        create_info.imageFormat = format.format;
        create_info.imageColorSpace = format.colorSpace;
        create_info.imageExtent = current_extent;

        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        std::int32_t queue_family_indices[] = {
            logical_device.get_physical_device().get_graphics_queue_family_index(),
            logical_device.get_physical_device().get_present_queue_family_index()
        };

        if (&logical_device.get_graphics_queue() == &logical_device.get_present_queue()) {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.pQueueFamilyIndices = nullptr;
            create_info.queueFamilyIndexCount = 0;
        } else {
            auto families = reinterpret_cast<std::uint32_t*>(queue_family_indices);
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.pQueueFamilyIndices = families;
            create_info.queueFamilyIndexCount = 2;
        }

        create_info.preTransform = surface.get_capabilities().currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        create_info.presentMode = static_cast<VkPresentModeKHR>(presentation_mode);
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        if (VkResult error = vkCreateSwapchainKHR(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create swap chain!" };
        }

        create_swapchain_images();
        create_swapchain_depths(logical_device);
    }

    SwapChain::~SwapChain() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, handle, nullptr);
        }
    }

    SwapChain::SwapChain(SwapChain&& swap_chain) noexcept {
        swap(*this, swap_chain);
    }

    SwapChain& SwapChain::operator=(SwapChain&& swap_chain) noexcept {
        swap(*this, swap_chain);
        return *this;
    }

    void swap(SwapChain& lhs, SwapChain& rhs) {
        using std::swap;

        swap(lhs.handle,  rhs.handle);
        swap(lhs.device,  rhs.device);
        swap(lhs.surface, rhs.surface);

        swap(lhs.images, rhs.images);
        swap(lhs.image_views, rhs.image_views);

        swap(lhs.format, rhs.format);
        swap(lhs.presentation_mode, rhs.presentation_mode);
        swap(lhs.current_extent, rhs.current_extent);
    }

    VkSwapchainKHR& SwapChain::get_handle() {
        return handle;
    }

    Surface& SwapChain::get_surface() const {
        return *surface;
    }

    std::uint32_t SwapChain::acquire_next_image(Fence& fence) {
        std::uint32_t next;
        vkAcquireNextImageKHR(device, handle, std::numeric_limits<std::uint64_t>::max(),
                              VK_NULL_HANDLE,
                              fence.get_handle(), &next);
        return next;
    }

    std::uint32_t SwapChain::acquire_next_image(Semaphore& semaphore) {
        std::uint32_t next;
        vkAcquireNextImageKHR(device, handle, std::numeric_limits<std::uint64_t>::max(),
                              semaphore.get_handle(),
                              VK_NULL_HANDLE, &next);
        return next;
    }

    std::vector<Framebuffer> SwapChain::create_framebuffers(RenderPass& render_pass) {
        std::vector<Framebuffer> framebuffers;

        framebuffers.reserve(image_views.size());

        for (auto& image_attachment : image_views) {
            if (render_pass.has_depth_attachment()) {
                framebuffers.emplace_back(device,
                                          render_pass,
                                          image_attachment,
                                          depth_buffer_view,
                                          get_extent());
            } else {
                framebuffers.emplace_back(device,
                                          render_pass,
                                          image_attachment,
                                          get_extent());
            }
        }

        return framebuffers;
    }

    std::vector<ImageView>& SwapChain::get_image_views() {
        return image_views;
    }

    VkImageView SwapChain::get_attachment(std::size_t i) {
        return image_views[i].get_handle();
    }

    const VkExtent2D& SwapChain::get_extent() const {
        return current_extent;
    }

    std::uint32_t SwapChain::get_width()  const {
        return current_extent.width;
    }

    std::uint32_t SwapChain::get_height() const {
        return current_extent.height;
    }

    float SwapChain::get_aspect_ratio() const {
        return get_width() / static_cast<float>(get_height());
    }

    const VkSurfaceFormatKHR& SwapChain::get_surface_format() const {
        return format;
    }

    const VkColorSpaceKHR& SwapChain::get_color_space() const {
        return format.colorSpace;
    }

    VkImageLayout SwapChain::get_khr_presentation_layout() const {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    VkImageLayout SwapChain::get_color_attachment_layout() const {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    VkFormat SwapChain::get_color_attachment_format() const {
        return format.format;
    }

    VkImageLayout SwapChain::get_depth_attachment_layout() const {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkFormat SwapChain::get_depth_attachment_format() const {
        return VK_FORMAT_D32_SFLOAT;
    }

    VkSampleCountFlagBits SwapChain::get_sample_count() {
        return VK_SAMPLE_COUNT_1_BIT; // TODO: 8x MSAA.
    }

    const SwapChain::PresentationMode& SwapChain::get_presentation_mode() const {
        return presentation_mode;
    }

    std::uint32_t SwapChain::size() const {
        return static_cast<std::uint32_t>(image_views.size());
    }

    void SwapChain::create_swapchain_images() {
        std::uint32_t image_count = images.size();
        vkGetSwapchainImagesKHR(device, handle, &image_count, nullptr);
        images.resize(image_count);
        vkGetSwapchainImagesKHR(device, handle, &image_count, images.data());

        image_views.reserve(image_count);

        for (const auto& image : images) {
            VkImageViewCreateInfo create_info;
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.pNext = nullptr;
            create_info.flags = 0;

            create_info.image = image;
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = format.format;

            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            VkImageView view;

            if (VkResult error = vkCreateImageView(device, &create_info, nullptr, &view)) {
                throw Exception { error, "couldn't create the swap chain image views!" };
            }

            image_views.emplace_back(ImageView { device, view });
        }
    }

    void SwapChain::create_swapchain_depths(Device& device) {
        depth_buffer_image = Image {
            device,
            get_width(), get_height(),
            VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        };

        depth_buffer_memory = DeviceMemory {
            device,
            depth_buffer_image.get_memory_requirements(),
            DeviceMemory::Type::DeviceLocal
        };

        depth_buffer_image.bind(depth_buffer_memory);

        depth_buffer_view = ImageView {
            device, depth_buffer_image
        };
    }

    void SwapChain::choose_extent(const VkExtent2D& window_extent) {
        const auto& capabilities= surface->get_capabilities();
        if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
            current_extent = surface->get_capabilities().currentExtent;
        } else {
            current_extent = window_extent;
            current_extent.width = std::max(capabilities.minImageExtent.width,
                                   std::min(capabilities.maxImageExtent.width,
                                   current_extent.width));
            current_extent.height = std::max(capabilities.minImageExtent.height,
                                    std::min(capabilities.maxImageExtent.height,
                                    current_extent.height));
        }
    }

    bool SwapChain::choose_format(const VkSurfaceFormatKHR& preferred_format) {
        bool found_format { false };

        const auto& formats = surface->get_formats();

        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            format = preferred_format;
            return true;
        }

        for (const auto& available_format : formats) {
            if (available_format.format     == preferred_format.format &&
                available_format.colorSpace == preferred_format.colorSpace) {
                format = preferred_format;
                return true;
            } else if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM) {
                format = available_format;
                found_format = true;
            }
        }

        return found_format;
    }

    bool SwapChain::choose_mode(const PresentationMode& preferred_presentation_mode) {
        const auto& present_modes = surface->get_presentation_modes();

        for (const auto& present_mode : present_modes) {
            auto mode = static_cast<PresentationMode>(present_mode);
            if (mode == preferred_presentation_mode) {
                presentation_mode = preferred_presentation_mode;
                return true;
            } else if (mode == PresentationMode::Fifo) {
                presentation_mode = mode;
            }
        }

        return true; // TODO: warn about other present mode.
    }
}
