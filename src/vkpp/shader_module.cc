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
        std::string ext { file_path.substr(file_path.find_last_of(".") + 1) };

        if (ext == "vert") {
            shader_type = Type::Vertex;
        } else if (ext == "tesc") {
            shader_type = Type::TesselationControl;
        } else if (ext == "tese") {
            shader_type = Type::TesselationEvaluation;
        } else if (ext == "frag") {
            shader_type = Type::Fragment;
        } else if (ext == "comp") {
            shader_type = Type::Compute;
        } else {
            throw Exception { "couldn't create shader module!",
            "the shader at '" + file_path + "' isn't a stage" };
        }

        spirv = load(file_path + ".spv");

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
        swap(lhs.file_path,    rhs.file_path);
        swap(lhs.file_size,    rhs.file_size);
        swap(lhs.hashed_spirv, rhs.hashed_spirv);
        swap(lhs.spirv,        rhs.spirv);
    }

    VkShaderModule& ShaderModule::get_handle() {
        return handle;
    }

    bool ShaderModule::recompile() {
        std::string compiler { VKPP_SHADER_MODULE_COMPILER };
        std::string shader_file { get_file_path() };
        compiler.append(" -o " + shader_file + ".spv ");
        std::string glslc_compile { compiler + shader_file };

#ifndef WINDOWS
        system(glslc_compile.c_str());
#else
        PROCESS_INFORMATION process_info {   };
        STARTUPINFOW startup_info {   };
        startup_info.cb = sizeof(startup_info);

        auto wide_glslc_cmd = to_lpcwstr(glslc_compile);

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

        auto spirv_candidate = load(shader_file + ".spv");
        auto hash = djb2a(spirv_candidate);

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
