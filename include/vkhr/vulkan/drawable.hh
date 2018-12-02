#ifndef VKHR_VULKAN_DRAWABLE_HH
#define VKHR_VULKAN_DRAWABLE_HH

#include <vkhr/vulkan/pipeline.hh>
#include <vkhr/vulkan/depth_map.hh>

#include <vkpp/command_buffer.hh>
#include <vkpp/buffer.hh>
#include <vkpp/descriptor_set.hh>

namespace vkhr {
    namespace vulkan {
        class Drawable {
        public:
            virtual ~Drawable() noexcept = default;
            virtual void draw(vkpp::CommandBuffer&) = 0;
        };
    }
}

#endif