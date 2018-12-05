#ifndef VKHR_BILLBOARD_HH
#define VKHR_BILLBOARD_HH

#include <vkhr/image.hh>

namespace vkhr {
    class Billboard final {
    public:
        Billboard() = default;
        Billboard(const std::string& file_path);

        operator bool() const;

        bool load(const std::string& file_path);

        vkhr::Image& get_image() const;

    private:
        bool success { false };
        mutable vkhr::Image image;
    };
}

#endif
