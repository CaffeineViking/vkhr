#include <vkpp/descriptor_set.hh>

#include <vkpp/device.hh>

#include <vkpp/debug_marker.hh>

#include <vkpp/exception.hh>

#include <utility>

namespace vkpp {
    DescriptorSet::Layout::Layout(Device& logical_device)
                                 : device { logical_device.get_handle() } {
        VkDescriptorSetLayoutCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.bindingCount = 0;
        create_info.pBindings    = nullptr;

        if (VkResult error = vkCreateDescriptorSetLayout(device, &create_info,
                                                         nullptr, &handle)) {
            throw Exception { error, "couldn't create descriptor set layout!" };
        }
    }

    DescriptorSet::Layout::Layout(Device& logical_device,
                                  const std::vector<DescriptorSet::Binding>& bindings)
                                 : device { logical_device.get_handle() },
                                   bindings { bindings } {
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings;

        layout_bindings.reserve(bindings.size());

        for (const auto& binding : bindings) {
            VkDescriptorSetLayoutBinding layout_binding {
                binding.id,
                binding.type,
                binding.count,
                VK_SHADER_STAGE_ALL,
                nullptr
            };

            layout_bindings.push_back(layout_binding);
        }

        VkDescriptorSetLayoutCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.bindingCount = layout_bindings.size();
        create_info.pBindings    = layout_bindings.data();

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
    }

    VkDescriptorSetLayout& DescriptorSet::Layout::get_handle() {
        return handle;
    }

    const std::vector<DescriptorSet::Binding>& DescriptorSet::Layout::get_bindings() const {
        return bindings;
    }

    DescriptorSet::Binding& DescriptorSet::Layout::get_binding(std::uint32_t binding_id) {
        for (auto& binding : bindings) {
            if (binding.id == binding_id)
                return binding;
        }

        throw Exception { "couldn't get descriptor set binding!", "no such binding ID!" };
    }

    DescriptorSet::DescriptorSet(VkDescriptorSet& descriptor_set,
                                 VkDescriptorPool& descriptor_pool,
                                 Layout* layout,
                                 VkDevice& device)
                                : handle { descriptor_set },
                                  pool   { descriptor_pool },
                                  layout { layout },
                                  device { device } {  }

    DescriptorSet::~DescriptorSet() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkFreeDescriptorSets(device, pool, 1, &handle);
        }
    }

    DescriptorSet::DescriptorSet(DescriptorSet&& pool) noexcept {
        swap(*this, pool);
    }

    DescriptorSet& DescriptorSet::operator=(DescriptorSet&& pool) noexcept {
        swap(*this, pool);
        return *this;
    }

    void swap(DescriptorSet& lhs, DescriptorSet& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.pool,   rhs.pool);
        swap(lhs.layout, rhs.layout);
        swap(lhs.device, rhs.device);
    }

    VkDescriptorSet& DescriptorSet::get_handle() {
        return handle;
    }

    DescriptorSet::Layout& DescriptorSet::get_layout() {
        return *layout;
    }

    void DescriptorSet::write(std::uint32_t binding,
                              Buffer& buffer,
                              VkDeviceSize offset,
                              VkDeviceSize range) {
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = buffer.get_handle();
        buffer_info.offset = offset;
        buffer_info.range = range;

        VkWriteDescriptorSet write_info;
        write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_info.pNext = nullptr;

        write_info.dstSet     = handle;
        write_info.dstBinding = binding;

        write_info.dstArrayElement = 0;

        write_info.descriptorCount = 1;
        write_info.descriptorType = layout->get_binding(binding).type;

        write_info.pImageInfo       = nullptr;
        write_info.pBufferInfo      = &buffer_info;
        write_info.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &write_info, 0, nullptr);
    }

    void DescriptorSet::write(std::uint32_t binding,
                              ImageView& image_view) {
        VkDescriptorImageInfo image_info;
        image_info.imageView = image_view.get_handle();
        image_info.imageLayout = image_view.get_layout();
        image_info.sampler = nullptr;

        VkWriteDescriptorSet write_info;
        write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_info.pNext = nullptr;

        write_info.dstSet     = handle;
        write_info.dstBinding = binding;

        write_info.dstArrayElement = 0;

        write_info.descriptorCount = 1;
        write_info.descriptorType = layout->get_binding(binding).type;

        write_info.pImageInfo       = &image_info;
        write_info.pBufferInfo      = nullptr;
        write_info.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &write_info, 0, nullptr);
    }

    void DescriptorSet::write(std::uint32_t binding,
                              ImageView& image_view,
                              Sampler& sampler) {
        VkDescriptorImageInfo image_info;
        image_info.imageView = image_view.get_handle();
        image_info.imageLayout = image_view.get_layout();
        image_info.sampler = sampler.get_handle();

        VkWriteDescriptorSet write_info;
        write_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_info.pNext = nullptr;

        write_info.dstSet     = handle;
        write_info.dstBinding = binding;

        write_info.dstArrayElement = 0;

        write_info.descriptorCount = 1;
        write_info.descriptorType = layout->get_binding(binding).type;

        write_info.pImageInfo       = &image_info;
        write_info.pBufferInfo      = nullptr;
        write_info.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &write_info, 0, nullptr);
    }

    DescriptorPool::DescriptorPool(Device& logical_device,
                                   const std::vector<VkDescriptorPoolSize>& pools)
                                  : pool_sizes { pools },
                                    device { logical_device.get_handle() } {
        VkDescriptorPoolCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        create_info.poolSizeCount = pools.size();
        create_info.pPoolSizes = pool_sizes.data();

        std::uint32_t total_descriptor_count = 0;

        for (const auto& pool : pools) {
            total_descriptor_count += pool.descriptorCount;
        }

        create_info.maxSets = total_descriptor_count;

        if (VkResult error = vkCreateDescriptorPool(device, &create_info,
                                                    nullptr, &handle)) {
            throw Exception { error, "couldn't create descriptor pool!" };
        }

        DebugMarker::object_name(device, *this, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Descriptor Pool");
    }

    DescriptorPool::~DescriptorPool() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, handle, nullptr);
        }
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& pool) noexcept {
        swap(*this, pool);
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& pool) noexcept {
        swap(*this, pool);
        return *this;
    }

    void swap(DescriptorPool& lhs, DescriptorPool& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.pool_sizes, rhs.pool_sizes);
        swap(lhs.device, rhs.device);
    }

    VkDescriptorPool& DescriptorPool::get_handle() {
        return handle;
    }

    const std::vector<VkDescriptorPoolSize>& DescriptorPool::get_pool_sizes() const {
        return pool_sizes;
    }

    DescriptorSet DescriptorPool::allocate(DescriptorSet::Layout& layout) {
        VkDescriptorSetAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.descriptorPool = handle;

        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout.get_handle();

        VkDescriptorSet ds;

        if (VkResult error = vkAllocateDescriptorSets(device, &alloc_info, &ds)) {
            throw Exception { error, "couldn't allocate descriptor set!" };
        }

        return DescriptorSet { ds, handle, &layout, device };
    }

    std::vector<DescriptorSet> DescriptorPool::allocate(std::uint32_t amount,
                                                        DescriptorSet::Layout& layout,
                                                        std::string name) {
        VkDescriptorSetAllocateInfo alloc_info;
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.pNext = nullptr;

        alloc_info.descriptorPool = handle;

        std::vector<VkDescriptorSetLayout> layouts(amount, layout.get_handle());

        alloc_info.descriptorSetCount = amount;
        alloc_info.pSetLayouts = layouts.data();

        std::vector<VkDescriptorSet> dss(amount);

        if (VkResult error = vkAllocateDescriptorSets(device, &alloc_info, dss.data())) {
            throw Exception { error, "couldn't allocate descriptor sets!" };
        }

        std::vector<DescriptorSet> descriptor_sets;

        descriptor_sets.reserve(dss.size());

        for (auto ds : dss) {
            descriptor_sets.emplace_back(
                ds, handle, &layout, device
            );

            if (!name.empty()) {
                DebugMarker::object_name(device, descriptor_sets.back(),
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET,
                                         name.c_str()); // for RenderDoc
            }
        }

        return descriptor_sets;
    }
}
