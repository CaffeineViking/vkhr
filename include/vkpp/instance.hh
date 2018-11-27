#ifndef VKPP_INSTANCE_HH
#define VKPP_INSTANCE_HH

#include <vkpp/application.hh>
#include <vkpp/debug_messenger.hh>
#include <vkpp/debug_marker.hh>
#include <vkpp/layer.hh>
#include <vkpp/extension.hh>
#include <vkpp/exception.hh>
#include <vkpp/physical_device.hh>
#include <vkpp/version.hh>

#include <vulkan/vulkan.h>

#include <vector>

namespace vkpp {
    class Instance final {
    public:
        Instance() = default;
        Instance(const Application& application_information,
                 const std::vector<Layer> required_layers,
                 const std::vector<Extension> required_extensions,
                 DebugMessenger::Callback debug_callback = nullptr);
        ~Instance() noexcept;

        Instance(Instance&& instance) noexcept;
        Instance& operator=(Instance&& instance) noexcept;

        friend void swap(Instance& lhs, Instance& rhs);

        VkInstance& get_handle();

        // Find the layers/extensions supported by this instance, returns those missing!
        template<typename T> std::vector<T> find(const std::vector<T>& requested,
                                                 const std::vector<T>& available) const;
        std::vector<Layer>     find(const std::vector<Layer>& layers) const;
        std::vector<Extension> find(const std::vector<Extension>& extensions) const;

        const Application& get_application() const;
        const std::vector<Layer>& get_enabled_layers() const;
        const std::vector<Extension>& get_enabled_extensions() const;

        // Finds physical devices which evaluates to true when 'suitable' is checked on it.
        template<typename F> const PhysicalDevice& find_physical_devices_with(F fun) const;

        const std::vector<PhysicalDevice>& get_physical_devices() const;

        DebugMessenger& get_debug_messenger();

        static Version get_api_version();
        static const std::vector<Layer>& get_available_layers();
        static const std::vector<Extension>& get_available_extensions();

        void label(VkDevice device);

    private:
        template<typename T> static std::string collapse(const std::vector<T>& vector);

        Application application_info;

        std::vector<Layer> enabled_layers;
        std::vector<Extension> enabled_extensions;
        std::vector<PhysicalDevice> physical_devices;
        static std::vector<Extension> available_extensions;
        static std::vector<Layer> available_layers;

        VkInstance handle { VK_NULL_HANDLE };
        DebugMessenger debug_utils_messenger;
    };

    template<typename T>
    std::string Instance::collapse(const std::vector<T>& vector) {
        std::string element_string;
        for (const auto element : vector)
            element_string.append(element.name + " ");
        return element_string;
    }

    template<typename T>
    std::vector<T> Instance::find(const std::vector<T>& requested,
                                  const std::vector<T>& available) const {
        std::vector<T> missing_requests;

        for (const auto request : requested) {
            bool request_found { false };
            for (const auto available_request : available) {
                if (request == available_request)
                    request_found = true;
            }

            if (!request_found)
                missing_requests.push_back(request);
        }

        return missing_requests;
    }

    template<typename F>
    const PhysicalDevice& Instance::find_physical_devices_with(F score) const {
        std::vector<int> scores;
        for (const auto& physical_device : physical_devices)
            scores.push_back(score(physical_device));

        int best_scoring { 0 };
        std::size_t best { 0 };
        for (std::size_t i { 0 }; i < physical_devices.size(); ++i) {
            if (scores[i] > best_scoring) {
                best_scoring = scores[i];
                best = i;
            }
        }

        if (best_scoring == 0) {
            throw Exception { "couldn't find the physical device!",
            "no physical devices achieve a score higher than 0!" };
        }

        return physical_devices[best];
    }
}

#endif
