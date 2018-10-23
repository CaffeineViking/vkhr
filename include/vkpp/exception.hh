#ifndef VKPP_EXCEPTION_HH
#define VKPP_EXCEPTION_HH

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <stdexcept>

namespace vkpp {
    class Exception : public std::runtime_error {
    public:
        Exception(const std::string& message)
                 : std::runtime_error { "Vulkan: " + message } {  }
        Exception(const std::string& message, const std::string& description)
                 : std::runtime_error { "Vulkan: " + message + "\n  Reason:  "
                                                   + description} {  }
        Exception(VkResult error, const std::string& message)
                 : std::runtime_error { "Vulkan " +
                                        error_map[error] +
                                        ": " + message } {  }
    private:
        static std::unordered_map<VkResult, std::string> error_map;
    };
}

#endif
