#ifndef VKPP_IMAGE_HH
#define VKPP_IMAGE_HH

#include <vulkan/vulkan.h>

namespace vkpp {
    class Image final {
    public:
    private:
        VkImage handle { VK_NULL_HANDLE };
    };

    class Device;

    class ImageView final {
    public:
        ImageView() = default;
        ImageView(VkDevice& device, VkImageView& image_view);

        ~ImageView() noexcept;

        ImageView(ImageView&& image_view) noexcept;
        ImageView& operator=(ImageView&& image_view) noexcept;

        friend void swap(ImageView& lhs, ImageView& rhs);

        VkImageView& get_handle();

    private:
        VkDevice device    { VK_NULL_HANDLE };
        VkImageView handle { VK_NULL_HANDLE };
    };
}

#endif
