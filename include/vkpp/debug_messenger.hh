#ifndef VKPP_DEBUG_MESSENGER_HH
#define VKPP_DEBUG_MESSENGER_HH

#include <vkpp/instance.hh>

#include <vulkan/vulkan.h>

namespace vkpp {
    class DebugMessenger final {
    public:
        enum class Severity {
            Verbose = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            Info = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
            Warning = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            Error = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        };

        DebugMessenger(Instance& instance, Severity severity = Severity::Warning);
        ~DebugMessenger() noexcept;

        DebugMessenger(DebugMessenger&& debug_messenger) noexcept;
        DebugMessenger& operator=(DebugMessenger&& debug_messenger) noexcept;

        friend void swap(DebugMessenger& lhs, DebugMessenger& rhs);

        void disable() { enabled = false; }
        bool is_enabled() const { return enabled; }
        void enable()  { enabled = true;  }

        void set_severity(Severity minimum_severity);
        Severity get_severity() const;

    private:
        static constexpr const char* CreateDebugUtilsMessenger {
            "vkCreateDebugUtilsMessengerEXT"
        };

        static constexpr const char* DestroyDebugUtilsMessenger {
            "vkDestroyDebugUtilsMessengerEXT"
        };

        bool enabled { true };
        Severity minimum_severity;
        VkDebugUtilsMessengerEXT handle;
        VkInstance instance_handle;
    };
}

#endif
