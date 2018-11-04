#include <vkpp/descriptor_set.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    DescriptorSet::Layout::Layout(Device& logical_device,
                                  const std::vector<DescriptorSet::Binding>& bindings)
                                 : device { logical_device.get_handle() },
                                   bindings { bindings } {
        std::vector<VkDescriptorSetLayoutBinding> real_bindings(bindings.size());
        for (const auto& binding : bindings) {
            VkDescriptorSetLayoutBinding real_binding {
                binding.id,
                binding.type,
                binding.count,
                VK_SHADER_STAGE_ALL,
                nullptr
            };

            real_bindings.push_back(real_binding);
        }

        VkDescriptorSetLayoutCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.bindingCount = real_bindings.size();
        create_info.pBindings    = real_bindings.data();

        if (VkResult error = vkCreateDescriptorSetLayout(device, &create_info,
                                                         nullptr, &handle)) {
            throw Exception { error, "couldn't create descriptor set layout!" };
        }
    }

    DescriptorSet::Layout::~Layout() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, handle, nullptr);
        }
    }

    DescriptorSet::Layout::Layout(Layout&& layout) noexcept {
        swap(*this, layout);
    }

    DescriptorSet::Layout& DescriptorSet::Layout::operator=(Layout&& layout) noexcept {
        swap(*this, layout);
        return *this;
    }

    void swap(DescriptorSet::Layout& lhs, DescriptorSet::Layout& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);
        swap(lhs.bindings, rhs.bindings);

        rhs.device = VK_NULL_HANDLE;
        rhs.handle = VK_NULL_HANDLE;
    }

    VkDescriptorSetLayout& DescriptorSet::Layout::get_handle() {
        return handle;
    }

    const std::vector<DescriptorSet::Binding>& DescriptorSet::Layout::get_bindings() const {
        return bindings;
    }
}
