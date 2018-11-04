#ifndef VKPP_QUEUE_HH
#define VKPP_QUEUE_HH

#include <vulkan/vulkan.h>

namespace vkpp {
    class Queue final {
    public:
        Queue() = default;

        Queue(const VkQueue& queue);

        VkQueue& get_handle();

        void submit();

        void wait_idle();

        void present();

    private:
        VkQueue handle { VK_NULL_HANDLE };
    };
}

#endif
