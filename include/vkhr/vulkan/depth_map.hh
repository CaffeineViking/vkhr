#ifndef VKHR_VULKAN_DEPTH_MAP_HH
#define VKHR_VULKAN_DEPTH_MAP_HH

#include <vulkan/vulkan.h>

namespace vkhr {
    namespace vulkan {
        class DepthMap final {
        public:
            VkFormat      get_attachment_format();
            VkImageLayout get_read_depth_layout();
            VkImageLayout get_attachment_layout();

        private:
        };
    }
}

#endif