#include <vkpp/queue.hh>

namespace vkpp {
    Queue::Queue(const VkQueue& queue)
                : handle { queue } { }

    VkQueue& Queue::get_handle() {
        return handle;
    }

    void Queue::submit() {
    }

    void Queue::wait_idle() {
        vkQueueWaitIdle(handle);
    }

    void Queue::present() {
    }
}
