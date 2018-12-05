#ifndef VKPP_DEBUG_MARKER_HH
#define VKPP_DEBUG_MARKER_HH

#include <vkpp/device.hh>
#include <vkpp/query.hh>

#include <vulkan/vulkan.h>

#include <cstdint>

#include <glm/glm.hpp>

namespace vkpp {
    class CommandBuffer;
    class DebugMarker final {
    public:
        static void setup_function_pointers(VkInstance instance);

        template<typename T>
        static void object_name(Device& device,   T& object, VkObjectType object_type, const char* name, int number = -1);

        template<typename T>
        static void object_name(VkDevice& device, T& object, VkObjectType object_type, const char* name, int number = -1);

        static void object_name(VkDevice& device, VkImage image_handle, VkObjectType object_type, const char* name);

        static void begin(CommandBuffer&  command_buffer, const char* name, const glm::vec4& = glm::vec4 { 0.0f });  
        static void insert(CommandBuffer& command_buffer, const char* name, const glm::vec4& = glm::vec4 { 0.0f });  
        static void end(CommandBuffer&    command_buffer);
        static void close(CommandBuffer&  command_buffer);

        static void set_current_query_pool(QueryPool& query_pool);

    private:
        static QueryPool* query_pool;
        static PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTagEXT;
        static PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
        static PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT; 
        static PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT;
        static PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT;
    };

    template<typename T>
    void DebugMarker::object_name(Device& device, T& object, VkObjectType object_type, const char* name, int number) {
        object_name(device.get_handle(), object, object_type, name, number);
    }

    template<typename T>
    void DebugMarker::object_name(VkDevice& device, T& object, VkObjectType object_type, const char* name, int number) {
        if (vkSetDebugUtilsObjectNameEXT) {
            VkDebugUtilsObjectNameInfoEXT name_info;
            name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            name_info.pNext = nullptr;

            name_info.objectHandle = reinterpret_cast<std::uint64_t>(object.get_handle());
            name_info.objectType = object_type;

            std::string stringed_name { name };

            if (number != -1) {
                stringed_name += " #";
                stringed_name += std::to_string(number);
            }

            name_info.pObjectName = stringed_name.c_str();

            vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }
    }
}

#endif