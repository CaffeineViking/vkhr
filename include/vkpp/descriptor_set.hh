#ifndef VKPP_DESCRIPTOR_SET_HH
#define VKPP_DESCRIPTOR_SET_HH

#include <vkpp/device.hh>

#include <vector>

namespace vkpp {
    class DescriptorSet final {
    public:
        struct Binding {
            std::uint32_t id;
            VkDescriptorType type;
            std::uint32_t count { 1 };
        };

        class Layout final {
        public:
            Layout() = default;
            Layout(Device& device,
                   const std::vector<DescriptorSet::Binding>& bindings);

            ~Layout() noexcept;

            Layout(Layout&& layout) noexcept;
            Layout& operator=(Layout&& layout) noexcept;

            friend void swap(Layout& lhs, Layout& rhs);

            VkDescriptorSetLayout& get_handle();

            const std::vector<DescriptorSet::Binding>& get_bindings() const;

        private:
            VkDevice device { VK_NULL_HANDLE };

            std::vector<DescriptorSet::Binding> bindings;

            VkDescriptorSetLayout handle { VK_NULL_HANDLE };
        };
    private:
    };
}

#endif
