#ifndef VKPP_IMAGE_HH
#define VKPP_IMAGE_HH

#include <vkhr/image.hh>
#include <vkpp/device_memory.hh>
#include <vkpp/buffer.hh>

#include <vulkan/vulkan.h>

#include <cstdint>

namespace vkpp {
    class Queue;
    class CommandPool;
    class Device;

    class Image {
    public:
        Image() = default;

        virtual ~Image() noexcept;

        Image(Device& device, std::uint32_t width, std::uint32_t height,
              VkFormat format, VkImageUsageFlags usage,
              std::uint32_t mip_levels = 1,
              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
              VkImageTiling tiling_mode = VK_IMAGE_TILING_OPTIMAL);

        Image(Image&& image) noexcept;
        Image& operator=(Image&& image) noexcept;

        friend void swap(Image& lhs, Image& rhs);

        VkImage& get_handle();

        const VkExtent2D& get_extent() const;
        VkFormat get_format() const;
        VkImageUsageFlags get_usage() const;
        std::uint32_t get_mip_levels() const;
        VkSampleCountFlagBits get_samples() const;
        VkSharingMode get_sharing_mode() const;
        VkImageTiling get_tiling_mode() const;

        VkDeviceMemory& get_bound_memory();
        VkMemoryRequirements get_memory_requirements() const;
        void bind(DeviceMemory& device_memory,
                  std::uint32_t offset = 0);

    private:
        VkExtent2D extent;
        VkFormat format;
        VkImageUsageFlags usage;
        std::uint32_t mip_levels;
        VkSampleCountFlagBits samples;

        VkImageTiling tiling_mode;

        VkSharingMode sharing_mode;

        VkDeviceMemory  memory { VK_NULL_HANDLE };

        VkDevice device { VK_NULL_HANDLE };
        VkImage handle  { VK_NULL_HANDLE };
    };

    class DeviceImage : public Image {
    public:
        DeviceImage() = default;

        friend void swap(DeviceImage& lhs, DeviceImage& rhs);
        DeviceImage& operator=(DeviceImage&& image) noexcept;
        DeviceImage(DeviceImage&& image) noexcept;

        DeviceImage(Device& device, CommandPool& pool,
                    vkhr::Image& image,
                    std::uint32_t mip_levels = 1);

        DeviceMemory& get_device_memory();

    private:
        void copy(Buffer& staged, CommandPool& pool);

        DeviceMemory device_memory;
    };

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
