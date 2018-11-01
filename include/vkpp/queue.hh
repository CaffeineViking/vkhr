#ifndef VKPP_QUEUE_HH
#define VKPP_QUEUE_HH

#include <vulkan/vulkan.h>

namespace vkpp {
    class Queue final {
    public:
        Queue(const VkQueue& queue);

    private:
        VkQueue handle;
    };
}

#endif
