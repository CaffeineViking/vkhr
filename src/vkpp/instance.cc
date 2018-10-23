#include <vkpp/instance.hh>

#include <vkpp/exception.hh>

namespace vkpp {
    Instance::Instance(const Application& application_information,
                       const std::vector<Layer> required_layers,
                       const std::vector<Extension> required_extensions)
                      : application_info { application_information },
                        enabled_layers { required_layers },
                        enabled_extensions { required_extensions } {
        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;

        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.pApplicationName = application_info.name.c_str();
        app_info.applicationVersion = application_info.app_version;
        app_info.pEngineName = application_info.engine_name.c_str();
        app_info.engineVersion = application_info.engine_version;
        app_info.apiVersion = application_info.api_version;

        auto missing_layers = find(required_layers);

        if (missing_layers.size() != 0) {
            auto missing = collapse(missing_layers);
            throw Exception { "couldn't create instance!",
            "the layers(s): " + missing + "are missing!"};
        }

        auto missing_extensions = find(required_extensions);

        if (missing_extensions.size() != 0) {
            auto missing = collapse(missing_extensions);
            throw Exception { "couldn't create instance!",
            "extension(s): " + missing + "are missing!"};
        }

        // Copy over the layer/extension names to packed name arrays.

        std::vector<const char*> layer_names(required_layers.size());
        for (std::size_t i { 0 }; i < layer_names.size(); ++i)
            layer_names[i] = required_layers[i].name.c_str();

        std::vector<const char*> extension_names(required_extensions.size());
        for (std::size_t i { 0 }; i < extension_names.size(); ++i)
            extension_names[i] = required_extensions[i].name.c_str();

        create_info.enabledExtensionCount = extension_names.size();
        create_info.ppEnabledExtensionNames = extension_names.data();
        create_info.enabledLayerCount = layer_names.size();
        create_info.ppEnabledLayerNames = layer_names.data();
        create_info.pApplicationInfo = &app_info;

        if (VkResult error = vkCreateInstance(&create_info, nullptr, &handle))
            throw Exception { error, "couldn't create instance!" };
    }

    Instance::~Instance() noexcept {
        vkDestroyInstance(handle, nullptr);
    }

    VkInstance& Instance::get_handle() {
        return handle;
    }

    std::vector<Layer> Instance::find(const std::vector<Layer>& layers) {
        std::vector<Layer> missing_layers;
        auto available_layers = get_available_layers();

        for (const auto layer : layers) {
            bool layer_found { false };
            for (const auto available_layer : available_layers) {
                if (layer == available_layer)
                    layer_found = true;
            }

            if (!layer_found) missing_layers.push_back(layer);
        }

        return missing_layers;
    }

    std::vector<Extension> Instance::find(const std::vector<Extension>& extensions) {
        std::vector<Extension> missing_extensions;
        auto available_extensions = get_available_extensions();

        for (const auto extension : extensions) {
            bool extension_found { false };
            for (const auto available_extension : available_extensions) {
                if (extension == available_extension)
                    extension_found = true;
            }

            if (!extension_found) missing_extensions.push_back(extension);
        }

        return missing_extensions;
    }

    const Application Instance::get_application_information() const {
        return application_info;
    }

    const std::vector<Layer>& Instance::get_enabled_layers() const {
        return enabled_layers;
    }

    const std::vector<Extension>& Instance::get_enabled_extensions() const {
        return enabled_extensions;
    }

    Version Instance::get_api_version() {
        std::uint32_t api_version;
        vkEnumerateInstanceVersion(&api_version);
        return api_version;
    }

    std::vector<Layer> Instance::get_available_layers() {
        std::uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> layer_properties(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

        std::vector<Layer> layers(layer_properties.size());
        for (std::size_t i { 0 }; i < layers.size(); ++i)
            layers[i] = layer_properties[i];

        return layers;
    }

    std::vector<Extension> Instance::get_available_extensions() {
        std::uint32_t extension_count;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> extension_properties(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count,
                                               extension_properties.data());

        std::vector<Extension> extensions(extension_properties.size());
        for (std::size_t i { 0 }; i < extensions.size(); ++i)
            extensions[i] = extension_properties[i];

        return extensions;
    }

    std::string Instance::collapse(const std::vector<Extension>& exts) {
        std::string extension_string;
        for (const auto extension : exts)
            extension_string.append(extension.name + " ");
        return extension_string;
    }

    std::string Instance::collapse(const std::vector<Layer>& layers) {
        std::string layer_string;
        for (const auto layer : layers)
            layer_string.append(layer.name + " ");
        return layer_string;
    }
}
