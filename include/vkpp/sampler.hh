#ifndef VKPP_SAMPLER_HH
#define VKPP_SAMPLER_HH

#include <vulkan/vulkan.h>

namespace vkpp {
    class Device;
    class Sampler final {
    public:
        ~Sampler() noexcept;

        Sampler() = default;

        Sampler(Device& device,
                VkFilter min_filter = VK_FILTER_LINEAR,
                VkFilter mag_filter = VK_FILTER_LINEAR,
                VkSamplerAddressMode wrap_u = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VkSamplerAddressMode wrap_v = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                bool anisotropy = true);

        Sampler(Sampler&& sampler) noexcept;
        Sampler& operator=(Sampler&& sampler) noexcept;

        friend void swap(Sampler& lhs, Sampler& rhs);

        VkSampler& get_handle();

        VkFilter get_min_filter() const;
        VkFilter get_mag_filter() const;

        VkSamplerAddressMode get_wrap_u() const;
        VkSamplerAddressMode get_wrap_v() const;

        bool anisotropy_enabled() const;

    private:
        VkFilter min_filter, mag_filter;
        VkSamplerAddressMode wrap_u, wrap_v;

        bool anisotropy { false };

        VkDevice  device { VK_NULL_HANDLE };
        VkSampler handle { VK_NULL_HANDLE };
    };
}

#endif
