#ifndef VKPP_QUEUE_HH
#define VKPP_QUEUE_HH

#include <vulkan/vulkan.h>

namespace vkpp {
    class Queue final {
    public:
        Queue() = default;
        Queue(const VkQueue& queue);

        operator VkQueue() const;

        VkQueue& get_handle();

    private:
        VkQueue handle { VK_NULL_HANDLE };
    };
}

#endif
