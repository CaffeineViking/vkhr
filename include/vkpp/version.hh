#ifndef VKPP_VERSION_HH
#define VKPP_VERSION_HH

#include <vulkan/vulkan.h>

#include <ostream>
#include <cstdint>

namespace vkpp {
    struct Version {
        Version() = default;
        Version(const std::uint32_t major,
                const std::uint32_t minor,
                const std::uint32_t patch = 0u);

        Version(const std::uint32_t version);

        operator std::uint32_t() const;

        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;
    };
}

std::ostream& operator<<(std::ostream& stream, const vkpp::Version& version);

#endif
