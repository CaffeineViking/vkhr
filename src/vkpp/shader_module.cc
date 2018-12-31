#include <vkpp/shader_module.hh>

#include <vkpp/device.hh>

#include <vkpp/exception.hh>

#ifndef   WINDOWS
#include <cstdlib>
#else
#include <windows.h>
#endif

#include <fstream>
#include <iostream>
#include <utility>

namespace vkpp {
    ShaderModule::ShaderModule(Device& logical_device,
                               const std::string& file_path)
                              : file_path { file_path },
                                device { logical_device.get_handle() } {
        file_extension = file_path.substr(file_path.find_last_of(".") + 1);
        file_name      = file_path.substr(0, file_path.find_first_of("."));

        if (file_extension == "vert") {
            shader_type = Type::Vertex;
        } else if (file_extension == "tesc") {
            shader_type = Type::TesselationControl;
        } else if (file_extension == "tese") {
            shader_type = Type::TesselationEvaluation;
        } else if (file_extension == "frag") {
            shader_type = Type::Fragment;
        } else if (file_extension == "comp") {
            shader_type = Type::Compute;
        } else if (file_extension == "hlsl") {
            shader_type = Type::Compute;
        } else {
            throw Exception { "couldn't create shader module!",
            "the shader at '" + file_path + "' isn't a stage" };
        }

        if (file_extension == "hlsl") {
            spirv = load(file_name + ".spv");
        } else {
            spirv = load(file_path + ".spv");
        }

        file_size = spirv.size();

        hashed_spirv = djb2a(spirv);

        VkShaderModuleCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        create_info.pCode = reinterpret_cast<const std::uint32_t*>(spirv.data());
        create_info.codeSize = spirv.size();

        if (VkResult error = vkCreateShaderModule(device, &create_info, nullptr, &handle)) {
            throw Exception { error, "couldn't create shader module!" };
        }
    }

    ShaderModule::ShaderModule(Device& device, const std::string& file_path,
                               const std::vector<VkSpecializationMapEntry>& constants,
                               void* constants_data, std::size_t constants_data_size)
                              : ShaderModule { device, file_path } {
        this->constants = constants;
        this->constants_data_size = constants_data_size;
        this->constants_data = constants_data;
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

        swap(lhs.shader_type,  rhs.shader_type);

        swap(lhs.file_path,      rhs.file_path);
        swap(lhs.file_name,      rhs.file_name);
        swap(lhs.file_extension, rhs.file_extension);

        swap(lhs.file_size,    rhs.file_size);
        swap(lhs.hashed_spirv, rhs.hashed_spirv);
        swap(lhs.spirv,        rhs.spirv);

        swap(lhs.constants, rhs.constants);
        swap(lhs.constants_data_size, rhs.constants_data_size);
        swap(lhs.constants_data, rhs.constants_data);
    }

    VkShaderModule& ShaderModule::get_handle() {
        return handle;
    }

    bool ShaderModule::recompile() {
        std::string compiler;
        std::string shader_module_path;

        if (file_extension == "hlsl") {
            compiler = VKPP_SHADER_MODULE_HLSLC;
            compiler.append(get_entry_point());
            compiler.append(" -c -o " + file_name + ".spv");
            shader_module_path = file_name + ".spv";
        } else {
            compiler = VKPP_SHADER_MODULE_GLSLC;
            compiler.append(" -o " + file_path + ".spv");
            shader_module_path = file_path + ".spv";
        }

        compiler.append(" " + file_path);

#ifndef WINDOWS
        system(compiler.c_str());
#else
        PROCESS_INFORMATION process_info {   };
        STARTUPINFOW startup_info {   };
        startup_info.cb = sizeof(startup_info);

        auto wide_glslc_cmd = to_lpcwstr(compiler);

        BOOL result = CreateProcessW(nullptr, (LPWSTR) wide_glslc_cmd.c_str(),
                                     nullptr, nullptr, false,
                                     NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW,
                                     nullptr, nullptr,
                                     &startup_info, &process_info);
        if (result) {
            WaitForSingleObject(process_info.hProcess, INFINITE);
            CloseHandle(process_info.hProcess);
            CloseHandle(process_info.hThread);
        }
#endif

        auto spirv_candidate = load(shader_module_path);
        auto hash            = djb2a(spirv_candidate);

        if (hash != hashed_spirv) {
            spirv = spirv_candidate;
            hashed_spirv = hash;
            file_size = spirv.size();

            VkShaderModuleCreateInfo create_info;
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.pNext = nullptr;
            create_info.flags = 0;

            create_info.pCode = reinterpret_cast<const std::uint32_t*>(spirv.data());
            create_info.codeSize = spirv.size();

            if (handle != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, handle, nullptr);
                handle = VK_NULL_HANDLE;
            }

            if (VkResult error = vkCreateShaderModule(device, &create_info,
                                                      nullptr, &handle)) {
                throw Exception { error, "couldn't create shader module!" };
            }

            return true;
        }

        return false;
    }

    ShaderModule::Type ShaderModule::get_stage() const {
        return shader_type;
    }

    std::size_t ShaderModule::get_file_size() const {
        return file_size;
    }

    const std::vector<char>& ShaderModule::get_spirv() const {
        return spirv;
    }

    std::uint32_t ShaderModule::get_hash() const {
        return hashed_spirv;
    }

    const std::string& ShaderModule::get_file_path() const {
        return file_path;
    }

    std::string ShaderModule::get_entry_point() const {
        if (file_extension == "hlsl")
            return file_name.substr(file_name.find_last_of("\\/") + 1);
        else
            return "main";
    }

    std::vector<char> ShaderModule::load(const std::string& file_path) {
        std::ifstream file { file_path, std::ios::ate |
                             std::ios::binary };

        if (!file) {
            throw Exception { "couldn't create shader module!",
            "the file path '" + file_path + "' doesn't exist" };
        }

        std::vector<char> blob;
        std::size_t size;

        size = file.tellg();
        blob.resize(size);
        file.seekg(0);
        file.read(blob.data(), size);
        file.close();

        return blob;
    }

    std::size_t ShaderModule::get_constants_data_size() const {
        return constants_data_size;
    }

    const std::vector<VkSpecializationMapEntry>& ShaderModule::get_constants() const {
        return constants;
    }

    const void* ShaderModule::get_constants_data() const {
        return constants_data;
    }

    std::uint32_t ShaderModule::djb2a(const std::vector<char>& data) {
        std::uint32_t hash { 5381 };

        for (auto byte : data) {
            hash = hash * 33 ^ static_cast<int>(byte);
        }

        return hash;
    }

    std::wstring ShaderModule::to_lpcwstr(const std::string& string) {
#ifndef WINDOWS
        return std::wstring { string.begin(), string.end() };
#else
        int len;
        int slength = (int)string.length() + 1;
        len = MultiByteToWideChar(CP_ACP, 0, string.c_str(), slength, 0, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, string.c_str(), slength, buf, len);
        std::wstring r(buf);
        delete[] buf;
        return r;
#endif
    }
}
