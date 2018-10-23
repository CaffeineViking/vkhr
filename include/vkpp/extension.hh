#ifndef VKPP_EXTENSION_HH
#define VKPP_EXTENSION_HH

#include <vkpp/version.hh>

#include <vulkan/vulkan.h>

#include <string>

namespace vkpp {
    struct Extension {
        Extension() = default;
        Extension(const std::string& name,
                  const Version& spec_version);

        Extension(const char* name);

        Extension(const VkExtensionProperties& extension);

        operator VkExtensionProperties() const;

        bool operator==(const Extension& other) const;
        bool operator!=(const Extension& other) const;

        std::string name;
        Version spec_version;
    };
}

#endif
