#ifndef VKPP_METADATA_HH
#define VKPP_METADATA_HH

#include <vkpp/version.hh>

#include <string>

namespace vkpp {
    struct Application {
        std::string name;
        Version app_version;
        std::string engine_name;
        Version engine_version;
        Version api_version;
    };
}

#endif
