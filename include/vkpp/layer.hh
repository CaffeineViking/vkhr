#ifndef VKPP_LAYER_HH
#define VKPP_LAYER_HH

#include <vkpp/version.hh>

#include <vulkan/vulkan.h>

#include <string>

namespace vkpp {
    struct Layer {
        Layer() = default;
        Layer(const std::string& name,
              const Version& spec_version,
              const Version& implementation_version,
              const std::string& description);

        Layer(const char* name);

        Layer(const VkLayerProperties& layer);

        operator VkLayerProperties() const;

        bool operator==(const Layer& other) const;
        bool operator!=(const Layer& other) const;

        std::string name;
        Version spec_version;
        Version implementation_version;
        std::string description;
    };
}

#endif
