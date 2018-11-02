#include <vkpp/queue.hh>

namespace vkpp {
    Queue::Queue(const VkQueue& queue)
                : handle { queue } {
    }

    Queue::operator VkQueue() const {
        return handle;
    }

    VkQueue& Queue::get_handle() {
        return handle;
    }
}
