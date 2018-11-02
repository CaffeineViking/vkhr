#include <vkpp/debug_messenger.hh>

#include <vkpp/exception.hh>

#include <iostream>

namespace vkpp {
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_messenger_callback(
                      VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                      VkDebugUtilsMessageTypeFlagsEXT,
                      const VkDebugUtilsMessengerCallbackDataEXT* message_data,
                      void* usrdata) {
        const auto minimum_severity = *reinterpret_cast<DebugMessenger::Severity*>(usrdata);
        const auto severity_bit = (VkDebugUtilsMessageSeverityFlagBitsEXT) minimum_severity;

        if (message_severity >= severity_bit) {
            std::cerr << '\n'
                      << message_data->pMessage
                      << std::endl;
        }

        return VK_FALSE;
    }

    DebugMessenger::DebugMessenger(VkInstance instance, Callback callback) {
        VkDebugUtilsMessengerCreateInfoEXT create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        if (callback == nullptr) {
            create_info.pfnUserCallback = debug_messenger_callback;
        } else {
            create_info.pfnUserCallback = callback;
        }

        this->minimum_severity = new Severity { Severity::Warning };
        create_info.pUserData = this->minimum_severity;

        auto fp = vkGetInstanceProcAddr(instance, pfn_create);
        auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) fp;

        if (vkCreateDebugUtilsMessengerEXT == nullptr) {
            throw Exception { "couldn't create debug messenger!",
            "vkCreateDebugUtilsMessengerEXT PFN doesn't exist!"};
        }

        if (VkResult error = vkCreateDebugUtilsMessengerEXT(instance, &create_info,
                                                            nullptr, &handle))
            throw Exception { error, "couldn't create debug messenger!" };
        this->instance_handle = instance; // Since we need to destroy *this it.
    }

    DebugMessenger::~DebugMessenger() noexcept {
        destroy();
    }

    void DebugMessenger::destroy() noexcept {
        auto fp = vkGetInstanceProcAddr(instance_handle, pfn_destroy);
        auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) fp;
        if (vkDestroyDebugUtilsMessengerEXT != nullptr && handle != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(instance_handle, handle, nullptr);
        }

        instance_handle = VK_NULL_HANDLE;
        handle = VK_NULL_HANDLE;

        if (minimum_severity != nullptr)
            delete minimum_severity;
        minimum_severity = nullptr;
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

        swap(lhs.minimum_severity, rhs.minimum_severity);
        swap(lhs.handle, rhs.handle);
        swap(lhs.instance_handle, rhs.instance_handle);

        rhs.instance_handle = VK_NULL_HANDLE;
        rhs.handle = VK_NULL_HANDLE;
    }

    DebugMessenger::Severity DebugMessenger::get_minimum_severity() const {
        return *minimum_severity;
    }

    void DebugMessenger::set_minimum_severity(Severity minimum_severity) {
        *(this->minimum_severity) = minimum_severity;
    }
}
