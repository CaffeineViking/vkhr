#include <vkhr/model.hh>

#include <iostream>

namespace vkhr {
    Model::Model(const std::string& file_path) {
        load(file_path);
    }

    bool Model::load(const std::string& file_path) {
        std::string error_msg { "" };
        success = tinyobj::LoadObj(&attributes, &shapes, &materials,
                                 &error_msg, file_path.c_str());
        if (!error_msg.empty()) std::cerr << error_msg << std::endl;
        if (!success) return false;

        success = true;
        return success;
    }

    Model::operator bool() const {
        return success;
    }

    const tinyobj::attrib_t& Model::get_attributes() const {
        return attributes;
    }

    const std::vector<tinyobj::material_t>& Model::get_materials() const {
        return materials;
    }

    const std::vector<tinyobj::shape_t>& Model::get_shapes() const {
        return shapes;
    }
}
