#include <vkpp/version.hh>

namespace vkpp {
    Version::Version(const std::uint32_t major,
                     const std::uint32_t minor,
                     const std::uint32_t patch)
                    : major { major },
                      minor { minor },
                      patch { patch } {  }

    Version::Version(const std::uint32_t version) {
        major = VK_VERSION_MAJOR(version);
        minor = VK_VERSION_MINOR(version);
        patch = VK_VERSION_PATCH(version);
    }

    Version::operator std::uint32_t() const {
        return VK_MAKE_VERSION(major,
                               minor,
                               patch);
    }
}

std::ostream& operator<<(std::ostream& stream, const vkpp::Version& version) {
    stream << version.major << '.' << version.minor << '.' << version.patch;
    return stream;
}
