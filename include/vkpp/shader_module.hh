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

        ShaderModule(ShaderModule&& shader_module) noexcept;
        ShaderModule& operator=(ShaderModule&& shader_module) noexcept;

        friend void swap(ShaderModule& lhs, ShaderModule& rhs);

        VkShaderModule& get_handle();

        std::size_t get_file_size() const;
        const std::string& get_file_path() const;
        const std::vector<char>& get_spirv() const;
        std::uint32_t get_hash() const;

    private:
        std::uint32_t djb2a(const std::vector<char>& data);

        std::string file_path;
        std::size_t file_size { 0 };
        std::uint32_t hashed_data;
        std::vector<char> spirv;

        VkDevice       device { VK_NULL_HANDLE };
        VkShaderModule handle { VK_NULL_HANDLE };
    };
}

#endif
