#include <vkpp/fence.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Fence::~Fence() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyFence(device, handle, nullptr);
        }
    }

    Fence::Fence(Device& logical_device) : device { logical_device.get_handle() } {
        VkFenceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        if (VkResult error = vkCreateFence(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create fence!" };
        }
    }

    Fence::Fence(Fence&& fence) noexcept {
        swap(*this, fence);
    }

    Fence& Fence::operator=(Fence&& fence) noexcept {
        swap(*this, fence);
        return *this;
    }

    void swap(Fence& lhs, Fence& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);

        lhs.handle = VK_NULL_HANDLE;
        lhs.device = VK_NULL_HANDLE;
    }

    VkFence& Fence::get_handle() {
        return handle;
    }

    bool Fence::await(std::uint64_t time) {
        return vkWaitForFences(device, 1, &handle, true, time) == VK_SUCCESS;
    }

    bool Fence::get_status() const {
        return vkGetFenceStatus(device, handle) == VK_SUCCESS;
    }

    void Fence::reset() {
        vkResetFences(device, 1, &handle);
    }
}
