#include <vkpp/image.hh>

#include <vkpp/device.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/queue.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Image::Image(Device& logical_device, std::uint32_t width, std::uint32_t height,
                 VkFormat format, VkImageUsageFlags usage, std::uint32_t mip_levels,
                 VkSampleCountFlagBits samples)
                : device { logical_device.get_handle() } {
        VkImageCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.imageType = VK_IMAGE_TYPE_2D;

        create_info.format = format;
        this->format = format;

        create_info.extent.width  = width;
        create_info.extent.height = height;
        create_info.extent.depth  = 1;

        this->extent = { width, height };

        this->mip_levels = mip_levels;
        create_info.mipLevels = mip_levels;
        create_info.arrayLayers = 1;

        create_info.samples = samples;
        this->samples = samples;

        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        create_info.usage = usage;
        this->usage = usage;

        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;

        create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        if (VkResult error = vkCreateImage(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create image!" };
        }
    }

    Image::~Image() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyImage(device, handle, nullptr);
        }
    }

    Image::Image(Image&& image) noexcept {
        swap(*this, image);
    }

    Image& Image::operator=(Image&& image) noexcept {
        swap(*this, image);
        return *this;
    }

    void swap(Image& lhs, Image& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);

        swap(lhs.extent, rhs.extent);
        swap(lhs.usage,  rhs.usage);
        swap(lhs.format, rhs.format);

        swap(lhs.mip_levels, rhs.mip_levels);
        swap(lhs.samples,    rhs.samples);
    }

    VkImage& Image::get_handle() {
        return handle;
    }

    const VkExtent2D& Image::get_extent() const {
        return extent;
    }

    VkFormat Image::get_format() const {
        return format;
    }

    VkImageUsageFlags Image::get_usage() const {
        return usage;
    }

    std::uint32_t Image::get_mip_levels() const {
        return mip_levels;
    }

    VkSampleCountFlagBits Image::get_samples() const {
        return samples;
    }

    VkDeviceMemory& Image::get_bound_memory() {
        return memory;
    }

    VkMemoryRequirements Image::get_memory_requirements() const {
        VkMemoryRequirements requirements;
        vkGetImageMemoryRequirements(device, handle, &requirements);
        return requirements;
    }

    void Image::bind(DeviceMemory& device_memory, std::uint32_t offset) {
        memory = device_memory.get_handle();
        vkBindImageMemory(device, handle, memory, offset);
    }

    void swap(DeviceImage& lhs, DeviceImage& rhs) {
        using std::swap;

        swap(static_cast<Image&>(lhs), static_cast<Image&>(rhs));

        swap(lhs.device_memory, rhs.device_memory);
    }

    DeviceImage& DeviceImage::operator=(DeviceImage&& image) noexcept {
        swap(*this, image);
        return *this;
    }

    DeviceImage::DeviceImage(DeviceImage&& image) noexcept {
        swap(*this, image);
    }

    DeviceImage::DeviceImage(Device& device, CommandPool& pool, vkhr::Image& image,
                             VkImageUsageFlags usage, std::uint32_t mip_levels,
                             VkSampleCountFlagBits samples)
                            : Image { device,
                                      static_cast<std::uint32_t>(image.get_width()),
                                      static_cast<std::uint32_t>(image.get_height()),
                                      VK_FORMAT_R8G8B8A8_UNORM, usage, mip_levels,
                                      samples } {
    }

    DeviceMemory& DeviceImage::get_device_memory() {
        return device_memory;
    }

    void DeviceImage::copy(Buffer& staged, CommandPool& pool) {
        // TODO: call vkCmdCopyBufferImage over here somehow.
    }

    ImageView::ImageView(VkDevice& device, VkImageView& image_view)
                        : device { device }, handle { image_view } { }

    ImageView::~ImageView() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyImageView(device, handle, nullptr);
        }
    }

    ImageView::ImageView(ImageView&& image_view) noexcept {
        swap(*this, image_view);
    }

    ImageView& ImageView::operator=(ImageView&& image_view) noexcept {
        swap(*this, image_view);
        return *this;
    }

    void swap(ImageView& lhs, ImageView& rhs) {
        using std::swap;

        swap(lhs.device, rhs.device);
        swap(lhs.handle, rhs.handle);
    }

    VkImageView& ImageView::get_handle() {
        return handle;
    }
}
