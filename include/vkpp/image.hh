#ifndef VKPP_IMAGE_HH
#define VKPP_IMAGE_HH

#include <vkhr/image.hh>
#include <vkpp/device_memory.hh>
#include <vkpp/sampler.hh>
#include <vkpp/buffer.hh>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <utility>

namespace vkpp {
    class Queue;
    class CommandPool;
    class CommandBuffer;
    class Device;

    class Image {
    public:
        Image() = default;

        virtual ~Image() noexcept;

        Image(Device& device, std::uint32_t width, std::uint32_t height,
              std::uint32_t depth, VkFormat format, VkImageUsageFlags usage,
              std::uint32_t mip_levels = 1,
              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
              VkImageTiling tiling_mode = VK_IMAGE_TILING_OPTIMAL);

        Image(Device& device, std::uint32_t width, std::uint32_t height,
              VkFormat format, VkImageUsageFlags usage,
              std::uint32_t mip_levels = 1,
              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
              VkImageTiling tiling_mode = VK_IMAGE_TILING_OPTIMAL);

        Image(VkDevice device, VkImage image,
              std::uint32_t width, std::uint32_t height, VkFormat format,
              VkImageUsageFlags usage, std::uint32_t mip_levels = 1,
              VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
              VkImageTiling tiling_mode = VK_IMAGE_TILING_OPTIMAL,
              bool destroy = true); // in case it's a swapchain...

        Image(Image&& image) noexcept;
        Image& operator=(Image&& image) noexcept;

        friend void swap(Image& lhs, Image& rhs);

        VkImage& get_handle();

        const VkExtent3D& get_extent() const;
        VkFormat get_format() const;
        VkImageUsageFlags get_usage() const;

        std::uint32_t get_mip_levels() const;
        VkSampleCountFlagBits get_samples() const;

        VkSharingMode get_sharing_mode() const;
        VkImageTiling get_tiling_mode() const;
        VkImageLayout get_layout() const;

        VkImageAspectFlags get_aspect_mask() const;

        void transition(CommandBuffer& command_buffer,
                        VkAccessFlags src_access, VkAccessFlags dst_access,
                        VkImageLayout src_layout, VkImageLayout dst_layout,
                        VkPipelineStageFlags src, VkPipelineStageFlags dst);

        void transition(CommandBuffer& command_buffer, VkImageLayout from, VkImageLayout to);
        void transition(CommandBuffer& command_buffer, VkImageLayout to);

        VkDeviceMemory& get_bound_memory();
        VkMemoryRequirements get_memory_requirements() const;
        void bind(DeviceMemory& device_memory,
                  std::uint32_t offset = 0);

    protected:
        VkExtent3D extent;
        VkFormat format;
        VkImageUsageFlags usage;
        std::uint32_t mip_levels;
        VkSampleCountFlagBits samples;

        VkImageAspectFlags aspect_mask;

        VkImageLayout layout;
        VkImageTiling tiling_mode;
        VkSharingMode sharing_mode;

        VkDeviceMemory  memory { VK_NULL_HANDLE };

        VkDevice device { VK_NULL_HANDLE };
        VkImage handle  { VK_NULL_HANDLE };

        bool destroy { true };
    };

    class DeviceImage : public Image {
    public:
        DeviceImage() = default;

        friend void swap(DeviceImage& lhs, DeviceImage& rhs);
        DeviceImage& operator=(DeviceImage&& image) noexcept;
        DeviceImage(DeviceImage&& image) noexcept;

        DeviceImage(Device& device, CommandPool& command_pool,
                    vkhr::Image& image,
                    std::uint32_t mip_levels = 1);

        DeviceImage(Device& device, std::uint32_t width, std::uint32_t height, VkDeviceSize size_in_bytes,
                    VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
                    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

        DeviceImage(Device& device,
                    std::uint32_t width, std::uint32_t height, std::uint32_t depth,
                    CommandPool& command_pool,
                    std::vector<unsigned char>& volume,
                    std::uint32_t mip_levels = 1);

        DeviceMemory& get_device_memory();

        Buffer& get_staging_buffer();

        DeviceMemory& get_staging_memory();

        void staged_copy(vkhr::Image& image,                 CommandBuffer& command_buffer);
        void staged_copy(std::vector<unsigned char>& volume, CommandBuffer& command_buffer);

    private:
        Buffer       staging_buffer;
        DeviceMemory staging_memory;

        DeviceMemory device_memory;
    };

    class ImageView final {
    public:
        ImageView() = default;
        ImageView(VkDevice& device, VkImageView& image_view);

        ImageView(Device& device, Image& image, VkImageLayout layout =  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        ~ImageView() noexcept;

        ImageView(ImageView&& image_view) noexcept;
        ImageView& operator=(ImageView&& image_view) noexcept;

        friend void swap(ImageView& lhs, ImageView& rhs);

        VkImage& get_image();

        VkImageLayout get_layout() const;

        VkImageView& get_handle();

    private:
        VkImageLayout layout { VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        VkImage image        { VK_NULL_HANDLE };
        VkDevice device      { VK_NULL_HANDLE };
        VkImageView handle   { VK_NULL_HANDLE };
    };

    using CombinedImageSampler = std::pair<ImageView, Sampler>;
}

#endif
