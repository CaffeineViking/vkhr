#include <vkpp/extension.hh>

#include <cstring>

namespace vkpp {
    Extension::Extension(const std::string& name,
                         const Version& spec_version)
                        : name { name },
                          spec_version { spec_version } {  }

    Extension::Extension(const char* name) : Extension { name, { 0, 0, 0 } } {  }

    Extension::Extension(const VkExtensionProperties& extension)
                        : name { extension.extensionName },
                          spec_version { extension.specVersion } {  }

    Extension::operator VkExtensionProperties() const {
        VkExtensionProperties extension_properties;
        std::strcpy(extension_properties.extensionName, name.c_str());
        extension_properties.specVersion = spec_version;
        return extension_properties;
    }

    bool Extension::operator==(const Extension& other) const {
        return name == other.name;
    }

    bool Extension::operator!=(const Extension& other) const {
        return !(*this == other);
    }
}
