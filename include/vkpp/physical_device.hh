#ifndef VKPP_PHYSICAL_DEVICE_HH
#define VKPP_PHYSICAL_DEVICE_HH

#include <vkpp/surface.hh>

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace vkpp {
    class PhysicalDevice final {
    public:
        PhysicalDevice(const VkPhysicalDevice& physical_device);

        operator VkPhysicalDevice() const;

        bool operator==(const PhysicalDevice& other) const;
        bool operator!=(const PhysicalDevice& other) const;

        VkPhysicalDevice& get_handle();

        enum class Type {
            Other = VK_PHYSICAL_DEVICE_TYPE_OTHER,
            IntegratedGpu = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
            DiscreteGpu = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
            VirtualGpu = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
            Cpu = VK_PHYSICAL_DEVICE_TYPE_CPU
        };

        bool is_discrete_gpu() const;
        bool is_integrated_gpu() const;
        bool is_virtual_gpu() const;

        std::uint32_t get_memory_heap() const;
        VkDeviceSize  get_memory_size() const;

        const VkPhysicalDeviceFeatures& get_features() const;
        const VkPhysicalDeviceMemoryProperties& get_memory_properties() const;
        const std::vector<VkQueueFamilyProperties> get_queue_families() const;
        const VkPhysicalDeviceProperties& get_properties() const;

        bool is_gpu() const;

        std::int32_t get_compute_queue() const;
        std::int32_t get_graphics_queue() const;
        std::int32_t get_transfer_queue() const;
        std::int32_t get_present_queue() const;

        std::string get_details() const;
        const std::string& get_name() const;
        Type get_type() const;
        std::string get_type_string() const;

    private:
        std::string name;

        Type type;

        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory_properties;
        VkPhysicalDeviceProperties properties;

        void find_device_memory_heap_index();

        std::vector<VkQueueFamilyProperties> queue_families;

        std::int32_t device_heap_index { -1 };

        void locate_queue_family_indices();

        std::int32_t graphics_queue_family_index { -1 };
        std::int32_t compute_queue_family_index  { -1 };
        std::int32_t transfer_queue_family_index { -1 };
        std::int32_t present_queue_family_index  { -1 };

        VkPhysicalDevice handle;
    };
}

#endif
