#include <vkhr/vulkan/depth_map.hh>

namespace vkhr {
    namespace vulkan {
        VkImageLayout DepthMap::get_read_depth_layout() {
            return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
        }

        VkFormat DepthMap::get_attachment_format() {
            return VK_FORMAT_D32_SFLOAT;
        }

        VkImageLayout DepthMap::get_attachment_layout() {
            return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }
}
