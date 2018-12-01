#ifndef VKPP_SEMAPHORE_HH
#define VKPP_SEMAPHORE_HH

#include <vulkan/vulkan.h>

#include <vector>

#include <cstdint>

namespace vkpp {
    class Device;
    class Semaphore final {
    public:
        Semaphore() = default;
        Semaphore(Device& device);

        ~Semaphore() noexcept;

        static std::vector<Semaphore> create(Device& device, std::uint32_t n = 1, const char* name = "");

        Semaphore(Semaphore&& semaphore) noexcept;
        Semaphore& operator=(Semaphore&& semaphore) noexcept;

        friend void swap(Semaphore& lhs, Semaphore& rhs);

        VkSemaphore& get_handle();

    private:
        VkDevice    device { VK_NULL_HANDLE };
        VkSemaphore handle { VK_NULL_HANDLE };
    };
}

#endif
