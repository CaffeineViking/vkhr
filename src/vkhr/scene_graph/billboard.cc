#include <vkhr/scene_graph/billboard.hh>

namespace vkhr {
    Billboard::Billboard(const std::string& file_path) {
        load(file_path);
    }

    Billboard::operator bool() const {
        return success;
    }

    bool Billboard::load(const std::string& file_path) {
        if (!image.load(file_path)) {
            success = false;
            return  success;
        }

        success = true;
        return success;
    }

    vkhr::Image& Billboard::get_image() const {
        return image;
    }
}