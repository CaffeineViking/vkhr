#include <vkpp/sampler.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    Sampler::~Sampler() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroySampler(device, handle, nullptr);
        }
    }

    Sampler::Sampler(Device& logical_device,
                     VkFilter min_filter,
                     VkFilter mag_filter,
                     VkSamplerAddressMode wrap_u,
                     VkSamplerAddressMode wrap_v,
                     VkSamplerAddressMode wrap_w,
                     bool anisotropy,
                     bool enable_compare_less_op)
                    : min_filter { min_filter },
                      mag_filter { mag_filter },
                      wrap_u { wrap_u },
                      wrap_v { wrap_v },
                      anisotropy { anisotropy },
                      device { logical_device.get_handle() } {
        VkSamplerCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.magFilter = mag_filter;
        create_info.minFilter = min_filter;

        create_info.addressModeU = wrap_u;
        create_info.addressModeV = wrap_v;
        create_info.addressModeW = wrap_w;

        create_info.anisotropyEnable = anisotropy;
        if (anisotropy)
            create_info.maxAnisotropy = 16;

        create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        create_info.mipLodBias = 0.0;
        create_info.minLod = 0.0;
        create_info.maxLod = 0.0;

        create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

        create_info.unnormalizedCoordinates = VK_FALSE;

        if (enable_compare_less_op) {
            create_info.compareEnable = VK_TRUE;
            create_info.compareOp = VK_COMPARE_OP_LESS;
        } else {
            create_info.compareEnable = VK_FALSE;
            create_info.compareOp = VK_COMPARE_OP_ALWAYS;
        }

        if (VkResult error = vkCreateSampler(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create texture sampler!" };
        }
    }

    Sampler::Sampler(Sampler&& sampler) noexcept {
        swap(*this, sampler);
    }

    Sampler& Sampler::operator=(Sampler&& sampler) noexcept {
        swap(*this, sampler);
        return *this;
    }

    void swap(Sampler& lhs, Sampler& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);

        swap(lhs.min_filter, rhs.min_filter);
        swap(lhs.mag_filter, rhs.mag_filter);

        swap(lhs.wrap_u, rhs.wrap_u);
        swap(lhs.wrap_v, rhs.wrap_v);

        swap(lhs.anisotropy, rhs.anisotropy);
    }

    VkSampler& Sampler::get_handle() {
        return handle;
    }

    VkFilter Sampler::get_min_filter() const {
        return min_filter;
    }

    VkFilter Sampler::get_mag_filter() const {
        return mag_filter;
    }

    VkSamplerAddressMode Sampler::get_wrap_u() const {
        return wrap_u;
    }

    VkSamplerAddressMode Sampler::get_wrap_v() const {
        return wrap_v;
    }

    bool Sampler::anisotropy_enabled() const {
        return anisotropy;
    }
}
