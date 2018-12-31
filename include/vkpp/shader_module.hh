#ifndef VKPP_SHADER_MODULE_HH
#define VKPP_SHADER_MODULE_HH

#define VKPP_SHADER_MODULE_GLSLC "glslc -O -g -c"
#define VKPP_SHADER_MODULE_HLSLC "glslc -O -g -fshader-stage=compute -fentry-point="

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace vkpp {
    class Device;
    class ShaderModule final {
    public:
        ShaderModule() = default;
        ShaderModule(Device& device,
                     const std::string& file_path);

        ShaderModule(Device& device, const std::string& shader_module_paths,
                     const std::vector<VkSpecializationMapEntry>& constants,
                     void* constants_data, std::size_t constants_data_size);

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

        bool recompile();

        Type get_stage() const;
        std::size_t get_file_size() const;
        const std::string& get_file_path() const;
        const std::vector<char>& get_spirv() const;
        std::uint32_t get_hash() const;
        
        std::string get_entry_point() const;

        const void* get_constants_data() const;
        const std::vector<VkSpecializationMapEntry>& get_constants() const;
        std::size_t get_constants_data_size() const;

    private:
        std::vector<char> load(const std::string& sbinary);
        std::uint32_t djb2a(const std::vector<char>& data);
        std::wstring to_lpcwstr(const std::string& string);

        Type shader_type;
        std::string file_path;
        std::string file_name;
        std::string file_extension;
        std::size_t file_size { 0 };
        std::uint32_t hashed_spirv;
        std::vector<char> spirv;

        void* constants_data { nullptr };
        std::vector<VkSpecializationMapEntry> constants;
        std::size_t constants_data_size { 0 };

        VkSpecializationInfo specialization_info;

        VkDevice       device { VK_NULL_HANDLE };
        VkShaderModule handle { VK_NULL_HANDLE };
    };
}

#endif
