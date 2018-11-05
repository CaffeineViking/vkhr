#include <vkpp/queue.hh>

#include <vkpp/exception.hh>

namespace vkpp {
    Queue::Queue(const VkQueue& queue, std::uint32_t family_index)
                : family_index { family_index }, handle { queue } { }

    VkQueue& Queue::get_handle() {
        return handle;
    }

    std::uint32_t Queue::get_family_index() const {
        return family_index;
    }

    void Queue::submit(CommandBuffer& command_buffer,
                       Semaphore& wait,
                       VkPipelineStageFlags wait_stage,
                       Semaphore& signal) {
        VkSubmitInfo submit_info {  };
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &wait.get_handle();

        submit_info.pWaitDstStageMask = &wait_stage;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer.get_handle();

        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signal.get_handle();

        if (VkResult error = vkQueueSubmit(handle, 1, &submit_info, VK_NULL_HANDLE)) {
            throw Exception { error, "couldn't submit command buffer to the queue!" };
        }
    }

    void Queue::wait_idle() {
        vkQueueWaitIdle(handle);
    }

    void Queue::present(SwapChain& swap_chain,
                        std::uint32_t indices,
                        Semaphore& wait) {
        VkPresentInfoKHR present_info {  };
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &wait.get_handle();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain.get_handle();
        present_info.pImageIndices = &indices;
        present_info.pResults = nullptr;

        if (VkResult error = vkQueuePresentKHR(handle, &present_info)) {
            throw Exception { error, "couldn't present the renders!" };
        }
    }
}
