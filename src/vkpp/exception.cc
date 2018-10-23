#include <vkpp/exception.hh>

namespace vkpp {
    std::unordered_map<VkResult, std::string> Exception::error_map {
        { VK_SUCCESS, "success" },
        { VK_NOT_READY, "not ready" },
        { VK_TIMEOUT, "timeout" },
        { VK_EVENT_SET, "event set" },
        { VK_EVENT_RESET, "event reset" },
        { VK_INCOMPLETE, "incomplete" },
        { VK_ERROR_OUT_OF_HOST_MEMORY, "error (out of host memory)" },
        { VK_ERROR_OUT_OF_DEVICE_MEMORY, "error (out of device memory)" },
        { VK_ERROR_INITIALIZATION_FAILED, "error (initialization failed)" },
        { VK_ERROR_DEVICE_LOST, "error (device lost)" },
        { VK_ERROR_MEMORY_MAP_FAILED, "error (memory map failed)" },
        { VK_ERROR_LAYER_NOT_PRESENT, "error (layer not present)" },
        { VK_ERROR_EXTENSION_NOT_PRESENT, "error (extension not present)" },
        { VK_ERROR_FEATURE_NOT_PRESENT, "error (feature not present)" },
        { VK_ERROR_INCOMPATIBLE_DRIVER, "error (incompatible driver)" },
        { VK_ERROR_TOO_MANY_OBJECTS, "error (too many objects)" },
        { VK_ERROR_FORMAT_NOT_SUPPORTED, "error (format not supported)" },
        { VK_ERROR_FRAGMENTED_POOL, "error (fragmented pool)" },
        { VK_ERROR_OUT_OF_POOL_MEMORY, "error (out of pool memory)" },
        { VK_ERROR_INVALID_EXTERNAL_HANDLE, "error (invalid external handle)" },
        { VK_ERROR_SURFACE_LOST_KHR, "error (surface lost)" },
        { VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "error (native window in use)" },
        { VK_SUBOPTIMAL_KHR, "suboptimal use" },
        { VK_ERROR_OUT_OF_DATE_KHR, "error (out of date)" },
        { VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "error (incompatible display)" },
        { VK_ERROR_VALIDATION_FAILED_EXT, "error (validation failed)" },
        { VK_ERROR_INVALID_SHADER_NV, "error (invalid shader)" },
        { VK_ERROR_FRAGMENTATION_EXT, "error (fragmentation)" },
        { VK_ERROR_NOT_PERMITTED_EXT, "error (not permitted)" },
        { VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "error (out of pool memory)" },
        { VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR, "error (invalid external handle"}
    };
}
