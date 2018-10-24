#ifndef VKPP_DEBUG_MESSENGER_HH
#define VKPP_DEBUG_MESSENGER_HH

#include <vulkan/vulkan.h>

#include <utility>

namespace vkpp {
    class DebugMessenger final {
    public:
        enum class Severity {
            Verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            Info = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
            Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            Error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        };

        using Callback = PFN_vkDebugUtilsMessengerCallbackEXT;

        static constexpr const char* validation_layer {
            "VK_LAYER_LUNARG_standard_validation"
        };

        static constexpr const char* debug_utils {
            "VK_EXT_debug_utils"
        };

        DebugMessenger() = default;
        DebugMessenger(VkInstance instance, Callback callback = nullptr);
        ~DebugMessenger() noexcept;

        void destroy() noexcept;

        DebugMessenger(DebugMessenger&& debug_messenger) noexcept;
        DebugMessenger& operator=(DebugMessenger&& debug_messenger) noexcept;

        friend void swap(DebugMessenger& lhs, DebugMessenger& rhs);

        void set_minimum_severity(Severity minimum_severity);
        Severity get_minimum_severity() const;

    private:
        static constexpr const char* pfn_create  { "vkCreateDebugUtilsMessengerEXT" };
        static constexpr const char* pfn_destroy { "vkDestroyDebugUtilsMessengerEXT" };

        Severity* minimum_severity { nullptr };
        VkDebugUtilsMessengerEXT handle { VK_NULL_HANDLE };
        VkInstance instance_handle;
    };
}

#endif
