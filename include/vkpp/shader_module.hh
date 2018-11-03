#ifndef VKPP_SHADER_MODULE_HH
#define VKPP_SHADER_MODULE_HH

#include <vkpp/device.hh>

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace vkpp {
    class ShaderModule final {
    public:
        ShaderModule() = default;
        ShaderModule(Device& device,
                     const std::string& file_path);

        ~ShaderModule() noexcept;

        enum class Type {
            Vertex = VK_SHADER_STAGE_VERTEX_BIT,
            TesselationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            TesselationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
            Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
            Compute = VK_SHADER_STAGE_COMPUTE_BIT
        };

        ShaderModule(ShaderModule&& shader_module) noexcept;
        ShaderModule& operator=(ShaderModule&& shader_module) noexcept;

        friend void swap(ShaderModule& lhs, ShaderModule& rhs);

        VkShaderModule& get_handle();

        Type get_stage() const;
        std::size_t get_file_size() const;
        const std::string& get_file_path() const;
        const std::vector<char>& get_spirv() const;
        std::uint32_t get_hash() const;

    private:
        std::uint32_t djb2a(const std::vector<char>& data);

        Type shader_type;
        std::string file_path;
        std::size_t file_size { 0 };
        std::uint32_t hashed_data;
        std::vector<char> spirv;

        // TODO: eventually also support this :-)
        VkSpecializationInfo specialization_info;

        VkDevice       device { VK_NULL_HANDLE };
        VkShaderModule handle { VK_NULL_HANDLE };
    };
}

#endif
