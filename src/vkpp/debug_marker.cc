#include <vkpp/debug_marker.hh>

#include <vkpp/command_buffer.hh>

#include <vkpp/exception.hh>

#include <cstring>

namespace vkpp {
    void DebugMarker::setup_function_pointers(VkInstance instance) {
        vkSetDebugUtilsObjectTagEXT  = (PFN_vkSetDebugUtilsObjectTagEXT)  vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
        if (vkSetDebugUtilsObjectTagEXT == nullptr) {
            throw Exception { "couldn't setup the debug marker!",
            "the vkSetDebugUtilsObjectTagEXT fn doesn't exist!"};
        }

        vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
        if (vkSetDebugUtilsObjectNameEXT == nullptr) {
            throw Exception { "couldn't setup the debug marker!",
            "the vkSetDebugUtilsObjectNameEXT fn doesn't exist!"};
        }

        vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT) vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
        if (vkCmdBeginDebugUtilsLabelEXT == nullptr) {
            throw Exception { "couldn't setup the debug marker!",
            "the vkCmdBeginDebugUtilsLabelEXT fn doesn't exist!"};
        }

        vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT) vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
        if (vkCmdInsertDebugUtilsLabelEXT == nullptr) {
            throw Exception { "couldn't setup the debug marker!",
            "the vkCmdInsertDebugUtilsLabelEXT doesn't exist!"};
        }

        vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT) vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
        if (vkCmdEndDebugUtilsLabelEXT == nullptr) {
            throw Exception { "couldn't setup the debug marker!",
            "the vkCmdEndDebugUtilsLabelEXT fn doesn't exist!"};
        }
    }

    void DebugMarker::object_name(VkDevice& device, VkImage image_handle, VkObjectType object_type, const char* name) {
        if (vkSetDebugUtilsObjectNameEXT) {
            VkDebugUtilsObjectNameInfoEXT name_info;
            name_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            name_info.pNext = nullptr;

            name_info.objectHandle = reinterpret_cast<std::uint64_t>(image_handle);
            name_info.objectType = object_type;

            name_info.pObjectName = name;

            vkSetDebugUtilsObjectNameEXT(device, &name_info);
        }
    }

    void DebugMarker::begin(CommandBuffer& command_buffer, const char* name, const glm::vec4& color) {
        if (vkCmdBeginDebugUtilsLabelEXT) {
            VkDebugUtilsLabelEXT label_info;
            label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label_info.pNext = nullptr;

            label_info.pLabelName = name;

            std::memcpy(label_info.color, &color[0], sizeof(color));

            vkCmdBeginDebugUtilsLabelEXT(command_buffer.get_handle(), &label_info);
        }
    }

    void DebugMarker::insert(CommandBuffer& command_buffer, const char* name, const glm::vec4& color) {
        if (vkCmdInsertDebugUtilsLabelEXT) {
            VkDebugUtilsLabelEXT label_info;
            label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label_info.pNext = nullptr;

            label_info.pLabelName = name;

            std::memcpy(label_info.color, &color[0], sizeof(color));

            vkCmdInsertDebugUtilsLabelEXT(command_buffer.get_handle(), &label_info);
        }
    }

    void DebugMarker::end(CommandBuffer& command_buffer) {
        if (vkCmdEndDebugUtilsLabelEXT) {
            vkCmdEndDebugUtilsLabelEXT(command_buffer.get_handle());
        }
    }

    void DebugMarker::close(CommandBuffer& command_buffer) {
        end(command_buffer);
    }

    PFN_vkSetDebugUtilsObjectTagEXT DebugMarker::vkSetDebugUtilsObjectTagEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT DebugMarker::vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkCmdBeginDebugUtilsLabelEXT DebugMarker::vkCmdBeginDebugUtilsLabelEXT = nullptr; 
    PFN_vkCmdEndDebugUtilsLabelEXT DebugMarker::vkCmdEndDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdInsertDebugUtilsLabelEXT DebugMarker::vkCmdInsertDebugUtilsLabelEXT = nullptr;

    QueryPool* DebugMarker::query_pool { nullptr };

    void DebugMarker::set_current_query_pool(QueryPool& query_pool_candidate) {
        query_pool = &query_pool_candidate;
    }
}