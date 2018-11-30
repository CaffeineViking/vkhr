#ifndef VKPP_DESCRIPTOR_SET_HH
#define VKPP_DESCRIPTOR_SET_HH

#include <vkpp/buffer.hh>
#include <vkpp/sampler.hh>
#include <vkpp/image.hh>

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

namespace vkpp {
    class Device;
    class DescriptorSet final {
    public:
        struct Binding {
            std::uint32_t id;
            VkDescriptorType type;
            VkShaderStageFlagBits stages { VK_SHADER_STAGE_ALL };
            std::uint32_t count { 1 };
        };

        DescriptorSet() = default;

        ~DescriptorSet() noexcept;

        DescriptorSet(DescriptorSet&& pool) noexcept;
        DescriptorSet& operator=(DescriptorSet&& pool) noexcept;

        friend void swap(DescriptorSet& lhs, DescriptorSet& rhs);

        VkDescriptorSet& get_handle();

        void write(std::uint32_t binding,
                   Buffer& buffer,
                   VkDeviceSize offset = 0,
                   VkDeviceSize size = VK_WHOLE_SIZE);

        void write(std::uint32_t binding,
                   ImageView& image_view,
                   Sampler& sampler);

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

            DescriptorSet::Binding& get_binding(std::uint32_t binding);

        private:
            VkDevice device { VK_NULL_HANDLE };

            std::vector<DescriptorSet::Binding> bindings;

            VkDescriptorSetLayout handle { VK_NULL_HANDLE };
        };

        DescriptorSet(VkDescriptorSet& descriptor_set,
                      VkDescriptorPool& descriptor_pool,
                      Layout* descriptor_set_layout,
                      VkDevice& device);

        Layout& get_layout();

    private:
        VkDescriptorSet  handle { VK_NULL_HANDLE };
        VkDescriptorPool pool   { VK_NULL_HANDLE };
        Layout*          layout { nullptr };
        VkDevice         device { VK_NULL_HANDLE };
    };

    class DescriptorPool final {
    public:
        DescriptorPool() = default;

        DescriptorPool(Device& device, const std::vector<VkDescriptorPoolSize>& pools);

        ~DescriptorPool() noexcept;

        DescriptorPool(DescriptorPool&& pool) noexcept;
        DescriptorPool& operator=(DescriptorPool&& pool) noexcept;

        friend void swap(DescriptorPool& lhs, DescriptorPool& rhs);

        VkDescriptorPool& get_handle();

        const std::vector<VkDescriptorPoolSize>& get_pool_sizes() const;

        DescriptorSet allocate(DescriptorSet::Layout& layout);
        std::vector<DescriptorSet> allocate(std::uint32_t count_of_sets,
                                            DescriptorSet::Layout& layout,
                                            std::string name = "");

    private:
        std::vector<VkDescriptorPoolSize> pool_sizes;

        VkDevice         device { VK_NULL_HANDLE };
        VkDescriptorPool handle { VK_NULL_HANDLE };
    };
}

#endif
