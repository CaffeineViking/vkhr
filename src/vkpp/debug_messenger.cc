#include <vkpp/debug_messenger.hh>

#include <vkpp/exception.hh>

#include <iostream>
#include <utility>

namespace vkpp {
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
                      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT,
                      const VkDebugUtilsMessengerCallbackDataEXT* message_data,
                      void* user_data) {
        DebugMessenger* debug_messenger = reinterpret_cast<DebugMessenger*>(user_data);
        bool debugger_enabled { debug_messenger->is_enabled() };
        auto severity = debug_messenger->get_severity();
        bool severe = message_severity >= (VkDebugUtilsMessageSeverityFlagBitsEXT)severity;
        if (debugger_enabled && severe)
            std::cerr << "Vulkan layers: " << message_data->pMessage << std::endl;
        return VK_FALSE;
    }

    DebugMessenger::DebugMessenger(Instance& instance, Severity severity)
                                  : minimum_severity { severity } {
        VkDebugUtilsMessengerCreateInfoEXT create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = this;

        auto fp = vkGetInstanceProcAddr(instance.get_handle(), CreateDebugUtilsMessenger);
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) fp;

        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            throw Exception { "couldn't create debug messenger!",
            "vkCreateDebugUtilsMessengerEXT PFN doesn't exist!"};
        }

        if (VkResult error = vkCreateDebugUtilsMessengerEXT(instance.get_handle(),
                                                            &create_info, nullptr,
                                                            &handle))
            throw Exception { error, "couldn't create debug messenger!" };

        this->instance_handle = instance.get_handle(); // We need this pointer to destroy.
    }

    DebugMessenger::~DebugMessenger() noexcept {
        auto fp = vkGetInstanceProcAddr(instance_handle, DestroyDebugUtilsMessenger);
        auto vkDestroyDebugUtilsMessengerEXT= (PFN_vkDestroyDebugUtilsMessengerEXT) fp;

        if (vkDestroyDebugUtilsMessengerEXT != nullptr && handle != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(instance_handle,
                                            handle, nullptr);
        } // If this fails we're doomed already. Just ignore.
    }

    DebugMessenger::DebugMessenger(DebugMessenger&& debug_messenger) noexcept {
        swap(*this, debug_messenger);
    }

    DebugMessenger& DebugMessenger::operator=(DebugMessenger&& debug_messenger) noexcept {
        swap(*this, debug_messenger);
        return *this;
    }

    void swap(DebugMessenger& lhs, DebugMessenger& rhs) {
        using std::swap;
        swap(lhs.enabled, rhs.enabled);
        swap(lhs.minimum_severity, rhs.minimum_severity);
        swap(lhs.handle, rhs.handle);
        swap(lhs.instance_handle, rhs.instance_handle);
        rhs.handle = VK_NULL_HANDLE;
    }

    DebugMessenger::Severity DebugMessenger::get_severity() const {
        return minimum_severity;
    }

    void DebugMessenger::set_severity(Severity minimum_severity) {
        this->minimum_severity = minimum_severity;
    }
}
