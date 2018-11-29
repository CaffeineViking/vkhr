#ifndef VKPP_FENCE_HH
#define VKPP_FENCE_HH

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace vkpp {
    class Device;
    class Fence final {
    public:
        Fence() = default;
        Fence(Device& device);

        ~Fence() noexcept;

        static std::vector<Fence> create(Device& device, std::uint32_t n = 1, const char* name = "");

        Fence(Fence&& fence) noexcept;
        Fence& operator=(Fence&& fence) noexcept;

        friend void swap(Fence& lhs, Fence& rhs);

        VkFence& get_handle();

        bool wait(std::uint64_t);

        bool is_signaled() const;

        void reset();

        bool wait_and_reset();

    private:
        VkDevice device { VK_NULL_HANDLE };
        VkFence  handle { VK_NULL_HANDLE };
    };
}

#endif
