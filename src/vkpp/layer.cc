#include <vkpp/layer.hh>

#include <cstring>

namespace vkpp {
    Layer::Layer(const std::string& name,
                 const Version& spec_version,
                 const Version& implementation_version,
                 const std::string& description)
                : name { name },
                  spec_version { spec_version },
                  implementation_version { implementation_version },
                  description { description } {  }

    Layer::Layer(const char* name) : Layer { name, { 0, 0, 0 }, { 0, 0, 0 }, "" } {  }

    Layer::Layer(const VkLayerProperties& layer)
                : name { layer.layerName },
                  spec_version { layer.specVersion },
                  implementation_version { layer.implementationVersion },
                  description { layer.description } {  }

    Layer::operator VkLayerProperties() const {
        VkLayerProperties layer_properties;
        std::strcpy(layer_properties.layerName, name.c_str());
        layer_properties.specVersion = spec_version;
        layer_properties.implementationVersion = implementation_version;
        std::strcpy(layer_properties.description, description.c_str());
        return layer_properties;
    }

    bool Layer::operator==(const Layer& other) const {
        return name == other.name;
    }

    bool Layer::operator!=(const Layer& other) const {
        return !(*this == other);
    }
}
