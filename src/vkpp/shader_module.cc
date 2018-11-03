#include <vkpp/shader_module.hh>

#include <vkpp/exception.hh>

#include <fstream>
#include <utility>

namespace vkpp {
    ShaderModule::ShaderModule(Device& logical_device,
                               const std::string& file_path)
                              : file_path { file_path },
                                device { logical_device.get_handle() } {
        std::ifstream file { file_path, std::ios::ate | std::ios::binary };

        if (!file) {
            throw Exception { "couldn't create shader module!",
            "the file path '" + file_path + "' doesn't exist" };
        }

        file_size = file.tellg();
        spirv.resize(file_size);
        file.seekg(0);
        file.read(spirv.data(), file_size);
        file.close();

        hashed_data = djb2a(spirv);

        VkShaderModuleCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.pCode = reinterpret_cast<const std::uint32_t*>(spirv.data());
        create_info.codeSize = file_size;

        if (VkResult error = vkCreateShaderModule(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create shader module!" };
        }
    }

    ShaderModule::~ShaderModule() noexcept {
        if (handle != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, handle, nullptr);
        }
    }

    ShaderModule::ShaderModule(ShaderModule&& shader_module) noexcept {
        swap(*this, shader_module);
    }

    ShaderModule& ShaderModule::operator=(ShaderModule&& shader_module) noexcept {
        swap(*this, shader_module);
        return *this;
    }

    void swap(ShaderModule& lhs, ShaderModule& rhs) {
        using std::swap;

        swap(lhs.handle, rhs.handle);
        swap(lhs.device, rhs.device);

        swap(lhs.file_path, rhs.file_path);

        rhs.handle = VK_NULL_HANDLE;
        rhs.device = VK_NULL_HANDLE;
    }

    VkShaderModule& ShaderModule::get_handle() {
        return handle;
    }

    std::size_t ShaderModule::get_file_size() const {
        return file_size;
    }

    const std::vector<char>& ShaderModule::get_spirv() const {
        return spirv;
    }

    std::uint32_t ShaderModule::get_hash() const {
        return hashed_data;
    }

    const std::string& ShaderModule::get_file_path() const {
        return file_path;
    }

    std::uint32_t ShaderModule::djb2a(const std::vector<char>& data) {
        std::uint32_t hash { 5381 };

        for (auto byte : data) {
            hash = hash * 33 ^ static_cast<int>(byte);
        }

        return hash;
    }
}
