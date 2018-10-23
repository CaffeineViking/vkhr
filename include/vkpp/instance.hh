#ifndef VKPP_INSTANCE_HH
#define VKPP_INSTANCE_HH

#include <vkpp/layer.hh>
#include <vkpp/metadata.hh>
#include <vkpp/extension.hh>
#include <vkpp/version.hh>

#include <vector>

namespace vkpp {
    class Instance final {
    public:
        Instance(const Application& application_information,
                 const std::vector<Layer> required_layers,
                 const std::vector<Extension> required_extensions);
        ~Instance() noexcept;

        VkInstance& get_handle();

        // Find if layer/extension is supported by this instance.
        // returns: the layers/extensions that are NOT supported!
        std::vector<Layer> find(const std::vector<Layer>& layers);
        std::vector<Extension> find(const std::vector<Extension>& extensions);

        const Application get_application_information() const;
        const std::vector<Layer>& get_enabled_layers() const;
        const std::vector<Extension>& get_enabled_extensions() const;

        static Version get_api_version();
        static std::vector<Layer> get_available_layers();
        static std::vector<Extension> get_available_extensions();

    private:
        static std::string collapse(const std::vector<Extension>& extensions);
        static std::string collapse(const std::vector<Layer>& layers);

        Application application_info;
        std::vector<Layer> enabled_layers;
        std::vector<Extension> enabled_extensions;

        VkInstance handle;
    };
}

#endif
