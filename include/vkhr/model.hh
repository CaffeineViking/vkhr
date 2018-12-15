#ifndef VKHR_MODEL_HH
#define VKHR_MODEL_HH

#include <tiny_obj_loader.h>

#include <string>
#include <glm/glm.hpp>
#include <vector>

namespace vkhr {
    class Model final {
    public:
        Model() = default;
        Model(const std::string& file_path);

        bool load(const std::string& file_path);

        operator bool() const;

        const tinyobj::attrib_t& get_attributes() const;
        const std::vector<tinyobj::material_t>& get_materials() const;
        const std::vector<tinyobj::shape_t>& get_shapes() const;

        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 texcoord;
        };

        const std::vector<Vertex>& get_vertices() const;
        const std::vector<std::uint32_t>& get_elements() const;

    private:
        bool success { false };
        tinyobj::attrib_t attributes;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::vector<std::uint32_t> elements;
        std::vector<Vertex> vertices;
    };
}

#endif
