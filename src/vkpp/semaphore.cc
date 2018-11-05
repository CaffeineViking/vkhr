#include <vkpp/semaphore.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Semaphore::~Semaphore() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, handle, nullptr);
        }
    }

    Semaphore::Semaphore(Device& logical_device) : device { logical_device.get_handle() } {
        VkSemaphoreCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        if (VkResult error = vkCreateSemaphore(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create semaphore!" };
        }
    }

    Semaphore::Semaphore(Semaphore&& semaphore) noexcept {
        swap(*this, semaphore);
    }

    Semaphore& Semaphore::operator=(Semaphore&& semaphore) noexcept {
        swap(*this, semaphore);
        return *this;
    }

    void swap(Semaphore& lhs, Semaphore& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);
    }

    VkSemaphore& Semaphore::get_handle() {
        return handle;
    }
}
