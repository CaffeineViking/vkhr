#ifndef VKHR_MODEL_HH
#define VKHR_MODEL_HH

#include <tiny_obj_loader.h>

#include <string>

namespace vkhr {
    class Model final {
    public:
        Model() = default;
        Model(const std::string& file_path);

        operator bool() const;
    };
}

#endif
