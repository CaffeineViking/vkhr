#include <vkpp/queue.hh>

namespace vkpp {
    Queue::Queue(const VkQueue& queue)
                : handle { queue } {
    }

    VkQueue& Queue::get_handle() {
        return handle;
    }
}
