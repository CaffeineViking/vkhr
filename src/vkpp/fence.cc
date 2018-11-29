#include <vkpp/fence.hh>

#include <vkpp/device.hh>

#include <vkpp/debug_marker.hh>

#include <vkpp/exception.hh>

#include <utility>
#include <limits>

namespace vkpp {
    Fence::~Fence() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyFence(device, handle, nullptr);
        }
    }

    std::vector<Fence> Fence::create(Device& device, std::uint32_t amount, const char* name) {
        std::vector<Fence> fences;
        fences.reserve(amount);
        for (std::uint32_t i { 0 }; i < amount; ++i) {
            fences.emplace_back(device);
            DebugMarker::object_name(device, fences.back(),
                                     VK_OBJECT_TYPE_FENCE,
                                     name);
        } return fences;
    }

    Fence::Fence(Device& logical_device) : device { logical_device.get_handle() } {
        VkFenceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

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
    }

    VkFence& Fence::get_handle() {
        return handle;
    }

    bool Fence::wait(std::uint64_t time) {
        return vkWaitForFences(device, 1, &handle, true, time) == VK_SUCCESS;
    }

    bool Fence::is_signaled() const {
        return vkGetFenceStatus(device, handle) == VK_SUCCESS;
    }

    void Fence::reset() {
        vkResetFences(device, 1, &handle);
    }

    bool Fence::wait_and_reset() {
        auto forever { std::numeric_limits<std::uint64_t>::max() };
        bool result { wait(forever) };
        reset(); // Put into signaled.
        return result;
    }
}
