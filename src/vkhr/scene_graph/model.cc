#include <vkhr/scene_graph/model.hh>

#include <iostream>

namespace vkhr {
    Model::Model(const std::string& file_path) {
        load(file_path);
    }

    bool Model::load(const std::string& file_path) {
        std::string error_msg { "" };
        std::string base_path { file_path.substr(0, file_path.find_last_of("\\/") + 1) };
        success = tinyobj::LoadObj(&attributes, &shapes, &materials,
                                   &error_msg, file_path.c_str(), base_path.c_str());
        if (!error_msg.empty()) std::cerr << error_msg << std::endl;
        if (!success) return false;

        std::size_t index_count { 0 };
        for (const auto& shape : shapes) {
            index_count += shape.mesh.indices.size();
        }

        vertices.reserve(index_count);
        elements.reserve(index_count);

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex;

                vertex.position = {
                    attributes.vertices[3 * index.vertex_index + 0],
                    attributes.vertices[3 * index.vertex_index + 1],
                    attributes.vertices[3 * index.vertex_index + 2]
                };

                vertex.texcoord = {
                           attributes.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attributes.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.normal = {
                    attributes.normals[3 * index.normal_index + 0],
                    attributes.normals[3 * index.normal_index + 1],
                    attributes.normals[3 * index.normal_index + 2]
                };

                vertices.push_back(vertex);
                elements.push_back(elements.size());
            }
        }

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

    const std::vector<Model::Vertex>& Model::get_vertices() const {
        return vertices;
    }

    const std::vector<std::uint32_t>& Model::get_elements() const {
        return elements;
    }
}
