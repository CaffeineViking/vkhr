#include <vkhr/hair_style.hh>

#include <cstring>
#include <algorithm>
#include <fstream>

namespace vkhr {
    HairStyle::HairStyle(const std::string& file_path) {
        load(file_path);
    }

    HairStyle::operator bool() const {
        return error_state == Error::None;
    }

    HairStyle::Error HairStyle::get_last_error_state() const {
        return error_state;
    }

    bool HairStyle::load(const std::string& file_path) {
        std::ifstream file { file_path, std::ios::binary };

        if (!file) return set_error_state(Error::OpeningFile);

        if (!file.read(reinterpret_cast<char*>(&file_header), sizeof(FileHeader)))
            return set_error_state(Error::ReadingFileHeader);

        if (!valid_signature()) return set_error_state(Error::InvalidSignature);

        if (!read_segments(file)) return set_error_state(Error::ReadingSegments);
        if (!read_vertices(file)) return set_error_state(Error::ReadingVertices);
        if (!read_thickness(file)) return set_error_state(Error::ReadingThickness);
        if (!read_transparancy(file)) return set_error_state(Error::ReadingTransparency);
        if (!read_color(file)) return set_error_state(Error::ReadingColor);
        if (!read_tangents(file)) return set_error_state(Error::ReadingTangents);
        if (!read_indices(file)) return set_error_state(Error::ReadingIndices);

        if (!format_is_valid()) return set_error_state(Error::InvalidFormat);

        return set_error_state(Error::None);
    }

    bool HairStyle::save(const std::string& file_path) const {
        complete_header(); // Fill in remaining header fields.

        if (!format_is_valid()) return set_error_state(Error::InvalidFormat);

        std::ofstream file { file_path, std::ios::binary };

        if (!file) return set_error_state(Error::OpeningFile);

        if (!file.write(reinterpret_cast<char*>(&file_header), sizeof(FileHeader)))
            return set_error_state(Error::WritingFileHeader);

        if (!write_segments(file)) return set_error_state(Error::WritingSegments);
        if (!write_vertices(file)) return set_error_state(Error::WritingVertices);
        if (!write_thickness(file)) return set_error_state(Error::WritingThickness);
        if (!write_transparancy(file)) return set_error_state(Error::WritingTransparency);
        if (!write_color(file)) return set_error_state(Error::WritingColor);
        if (!write_tangents(file)) return set_error_state(Error::WritingTangents);
        if (!write_indices(file)) return set_error_state(Error::WritingIndices);

        // Signature is already set, so we don't need to check for validity.

        return set_error_state(Error::None);
    }

    unsigned HairStyle::get_strand_count() const {
        if (segments.size() != 0) {
            return static_cast<unsigned>(segments.size());
        } else {
            // Use the manually defined one.
            return file_header.strand_count;
        }
    }

    unsigned HairStyle::get_segment_count() const {
        return get_vertex_count() - get_strand_count();
    }

    void HairStyle::set_strand_count(const unsigned strand_count) {
        file_header.strand_count = strand_count;
    }

    unsigned HairStyle::get_vertex_count() const {
        return static_cast<unsigned>(vertices.size());
    }

    bool HairStyle::has_segments() const { return segments.size(); }
    bool HairStyle::has_vertices() const { return vertices.size(); }
    bool HairStyle::has_thickness() const { return thickness.size(); }
    bool HairStyle::has_transparency() const { return transparency.size(); }
    bool HairStyle::has_color() const { return color.size(); }
    bool HairStyle::has_tangents() const { return tangents.size(); }
    bool HairStyle::has_indices() const { return indices.size(); }

    // Pre-generated AABB for the hair styles.
    bool HairStyle::has_bounding_box() const {
        return file_header.field.has_bounding_box;
    }

    // Below follows boilerplate for writing to the header.

    unsigned HairStyle::get_default_segment_count() const {
        return file_header.default_segment_count;
    }

    void HairStyle::set_default_segment_count(const unsigned default_segment_count) {
        file_header.default_segment_count = default_segment_count;
    }

    void HairStyle::set_default_thickness(const float default_thickness) {
        file_header.default_thickness = default_thickness;
    }

    float HairStyle::get_default_thickness() const {
        return file_header.default_thickness;
    }

    float HairStyle::get_default_transparency() const {
        return file_header.default_transparency;
    }

    void HairStyle::set_default_transparency(const float default_transparency) {
        file_header.default_transparency = default_transparency;
    }

    void HairStyle::set_default_color(const glm::vec3& default_color) {
        file_header.default_color[0] = default_color[0];
        file_header.default_color[1] = default_color[1];
        file_header.default_color[2] = default_color[2];
    }

    glm::vec3 HairStyle::get_default_color() const {
        return glm::vec3 { file_header.default_color[0],
                           file_header.default_color[1],
                           file_header.default_color[2] };
    }

    const char* HairStyle::get_information() const {
        return file_header.information;
    }

    void HairStyle::set_information(const std::string& information) {
        const std::size_t info_size { sizeof(file_header.information) };
        std::memset(file_header.information, '\0', info_size); // For consistency.
        const std::size_t copy_size { std::min(info_size, information.length()) };
        std::strncpy(file_header.information, information.c_str(), copy_size);
    }

    void HairStyle::generate_tangents() {
        tangents.reserve(get_vertex_count());

        std::size_t vertex { 0 };
        for (std::size_t strand { 0 }; strand < get_strand_count(); ++strand) {
            unsigned segment_count { get_default_segment_count() };

            if (has_segments()) segment_count = segments[strand];

            for (std::size_t segment { 0 }; segment < segment_count; ++segment) {
                const auto& current_vertex { vertices[vertex + 0] };
                const auto& next_vertex    { vertices[vertex + 1] };
                const auto tangent { next_vertex - current_vertex };

                tangents.push_back(glm::normalize(tangent));
                ++vertex;
            }

            tangents.push_back(tangents.back()); // Special:
            ++vertex; // must derive tangents from previous.
        }
    }

    void HairStyle::generate_indices() {
        indices.reserve(get_segment_count() * 2);

        std::size_t vertex { 0 };
        for (std::size_t strand { 0 }; strand < get_strand_count(); ++strand) {
            unsigned segment_count { get_default_segment_count() };

            if (has_segments()) segment_count = this->segments[strand];

            for (std::size_t segment { 0 }; segment < segment_count; ++segment) {
                indices.push_back(static_cast<unsigned>(vertex++));
                indices.push_back(static_cast<unsigned>(vertex));
            }

            ++vertex; // Skips the last one.
        }
    }

    void HairStyle::generate_bounding_box() {
        glm::vec3 min_aabb { 0.0f, 0.0f, 0.0f },
                  max_aabb { 0.0f, 0.0f, 0.0f };

        for (const auto& position : vertices) {
            min_aabb.x = glm::min(position.x, min_aabb.x);
            min_aabb.y = glm::min(position.y, min_aabb.y);
            min_aabb.z = glm::min(position.z, min_aabb.z);
            max_aabb.x = glm::max(position.x, max_aabb.x);
            max_aabb.y = glm::max(position.y, max_aabb.y);
            max_aabb.z = glm::max(position.z, max_aabb.z);
        }

        std::memcpy(&file_header.bounding_box_min[0],
                    &min_aabb[0], sizeof(min_aabb));
        std::memcpy(&file_header.bounding_box_max[0],
                    &max_aabb[0], sizeof(max_aabb));

        file_header.field.has_bounding_box = true;
    }

    AABB HairStyle::get_bounding_box() const {
        glm::vec3 origin {
            file_header.bounding_box_min[0],
            file_header.bounding_box_min[1],
            file_header.bounding_box_min[2]
        };

        glm::vec3 size {
             file_header.bounding_box_max[0] - file_header.bounding_box_min[0],
             file_header.bounding_box_max[1] - file_header.bounding_box_min[1],
             file_header.bounding_box_max[2] - file_header.bounding_box_min[2]
        };

        return AABB {
            origin,
            glm::length(size),
            size,
            size.x * size.y * size.z
        };
    }

    HairStyle::Volume HairStyle::voxelize_vertices(std::size_t width, std::size_t height, std::size_t depth) const {
        Volume volume {
            {
                width,
                height,
                depth
            },
            get_bounding_box()
        };

        volume.data.resize(width * height * depth, 0); // 256x256 ~16MiB
        glm::vec3 voxel_size { volume.bounds.size / volume.resolution };

        for (const auto& vertex : vertices) {
            glm::vec3 voxel { (vertex - volume.bounds.origin) / voxel_size };
            voxel = glm::min(glm::floor(voxel), volume.resolution - 1.0000f);
            std::size_t pos = voxel.x + voxel.y*width + voxel.z*width*height;
            if (volume.data[pos] != 255) ++volume.data[pos]; // 8-bit voxels.
        }

        return volume;
    }

    HairStyle::Volume HairStyle::voxelize_segments(std::size_t width, std::size_t height, std::size_t depth) const {
        Volume volume {
            {
                width,
                height,
                depth
            },
            get_bounding_box()
        };

        volume.data.resize(width * height * depth, 0); // 256x256 ~16MiB
        glm::vec3 voxel_size { volume.bounds.size / volume.resolution };

        for (std::size_t i { 0 }; i < indices.size() - 1; i += 2) {
            auto root { (vertices[indices[i]]     - volume.bounds.origin) / voxel_size };
            auto tip  { (vertices[indices[i + 1]] - volume.bounds.origin) / voxel_size };

            auto direction { tip - root };
            float steps { glm::compMax(glm::abs(direction)) };
            direction /= steps; // [-1, 1]

            while (steps-- > 0.0f) {
                auto voxel = glm::min(glm::floor(root), volume.resolution-1.0f);
                int voxel_index = voxel.x + voxel.y*width + voxel.z*width*height;
                if (volume.data[voxel_index] != 255) ++volume.data[voxel_index];
                root += direction; // Move to the voxel we're going to rasterize
            }
        }

        return volume;
    }

    HairStyle::Volume HairStyle::pregather_density(const Volume& volume) {
        return Volume {  };
    }

    bool HairStyle::Volume::save(const std::string& file_path) {
        std::ofstream file { file_path, std::ios::binary };
        if (!file) return false; // Couldn't write to file.

        // Write voxels in one go (assume they are laid out right).
        if (!file.write(reinterpret_cast<const char*>(data.data()),
                        data.size() * sizeof(data[0])))
            return false;

        return true;
    }

    std::vector<glm::vec4> HairStyle::create_position_thickness_data() const {
        std::vector<glm::vec4> position_thicknesses(get_vertex_count());
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < static_cast<int>(get_vertex_count()); ++i) {
            float thickness { get_default_thickness() };
            if (has_thickness()) {
                thickness = this->thickness[i];
            }

            position_thicknesses[i] = glm::vec4 {
                vertices[i],
                thickness
            };
        } return position_thicknesses;
    }

    std::vector<glm::vec4> HairStyle::create_color_transparency_data() const {
        std::vector<glm::vec4> color_transparencies(get_vertex_count());
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < static_cast<int>(get_vertex_count()); ++i) {
            float transparency { get_default_transparency() };
            if (has_transparency()) {
                transparency = this->transparency[i];
            }

            glm::vec3 color { get_default_color() };
            if (has_color()) {
                color = this->color[i];
            }

            color_transparencies[i] = glm::vec4 {
                color,
                transparency
            };
        } return color_transparencies;
    }

    const std::vector<unsigned>& HairStyle::get_indices() const {
        return indices;
    }

    const std::vector<glm::vec3>& HairStyle::get_tangents() const {
        return tangents;
    }

    const std::vector<float>& HairStyle::get_thickness() const {
        return thickness;
    }

    const std::vector<glm::vec3>& HairStyle::get_vertices() const {
        return vertices;
    }

    const std::vector<unsigned short>& HairStyle::get_segments() const {
        return segments;
    }

    const std::vector<float>& HairStyle::get_transparency() const {
        return transparency;
    }

    const std::vector<glm::vec3>& HairStyle::get_color() const {
        return color;
    }

    bool HairStyle::valid_signature() const {
        return file_header.signature[0] == 'H' &&
               file_header.signature[1] == 'A' &&
               file_header.signature[2] == 'I' &&
               file_header.signature[3] == 'R';
    }

    bool HairStyle::format_is_valid() const {
        if (!has_vertices()) return false;
        if (!valid_signature()) return false;
        if (has_thickness() && thickness.size() != vertices.size()) return false;
        if (has_transparency() && transparency.size() != vertices.size()) return false;
        if (has_color() && color.size() != vertices.size()) return false;
        return true; // The rest we assume is right. It's hard to verify.
    }

    void HairStyle::complete_header() const {
        file_header.signature[0] = 'H';
        file_header.signature[1] = 'A';
        file_header.signature[2] = 'I';
        file_header.signature[3] = 'R';

        update_bitfield();

        file_header.strand_count = get_strand_count();
        file_header.vertex_count = get_vertex_count();
    }

    void HairStyle::update_bitfield() const {
        file_header.field.has_segments = has_segments();
        file_header.field.has_vertices = has_vertices();
        file_header.field.has_thickness = has_thickness();
        file_header.field.has_transparency = has_transparency();
        file_header.field.has_color = has_color();
        file_header.field.has_tangents = has_tangents();
        file_header.field.has_indices = has_indices();
        file_header.field.future_extension = 0;
    }

    bool HairStyle::set_error_state(const Error error_state) const {
        this->error_state = error_state;
        if (error_state == Error::None) {
            return true;
        } else return false;
    }

    bool HairStyle::read_segments(std::ifstream& file) {
        if (file_header.field.has_segments) {
            segments.resize(file_header.strand_count);
            return read_field(file, segments);
        } return true;
    }

    bool HairStyle::read_vertices(std::ifstream& file) {
        if (file_header.field.has_vertices) {
            vertices.resize(file_header.vertex_count);
            return read_field(file, vertices);
        } return true;
    }

    bool HairStyle::read_thickness(std::ifstream& file) {
        if (file_header.field.has_thickness) {
            thickness.resize(file_header.vertex_count);
            return read_field(file, thickness);
        } return true;
    }

    bool HairStyle::read_transparancy(std::ifstream& file) {
        if (file_header.field.has_transparency) {
            transparency.resize(file_header.vertex_count);
            return read_field(file, transparency);
        } return true;
    }

    bool HairStyle::read_color(std::ifstream& file) {
        if (file_header.field.has_color) {
            color.resize(file_header.vertex_count);
            return read_field(file, color);
        } return true;
    }

    bool HairStyle::read_tangents(std::ifstream& file) {
        if (file_header.field.has_tangents) {
            tangents.resize(file_header.vertex_count);
            return read_field(file, tangents);
        } return true;
    }

    bool HairStyle::read_indices(std::ifstream& file) {
        if (file_header.field.has_indices) {
            indices.resize(get_segment_count() * 2);
            return read_field(file, indices);
        } return true;
    }

    bool HairStyle::write_segments(std::ofstream& file) const {
        if (file_header.field.has_segments) {
            return write_field(file, segments);
        } return true;
    }

    bool HairStyle::write_vertices(std::ofstream& file) const {
        if (file_header.field.has_vertices) {
            return write_field(file, vertices);
        } return true;
    }

    bool HairStyle::write_thickness(std::ofstream& file) const {
        if (file_header.field.has_thickness) {
            return write_field(file, thickness);
        } return true;
    }

    bool HairStyle::write_transparancy(std::ofstream& file) const {
        if (file_header.field.has_transparency) {
            return write_field(file, transparency);
        } return true;
    }

    bool HairStyle::write_color(std::ofstream& file) const {
        if (file_header.field.has_color) {
            return write_field(file, color);
        } return true;
    }

    bool HairStyle::write_tangents(std::ofstream& file) const {
        if (file_header.field.has_tangents) {
            return write_field(file, tangents);
        } return true;
    }

    bool HairStyle::write_indices(std::ofstream& file) const {
        if (file_header.field.has_indices) {
            return write_field(file, indices);
        } return true;
    }

    std::size_t HairStyle::get_size() const {
        std::size_t size_in_bytes { 0 };
        size_in_bytes += segments.size() * sizeof(segments[0]);
        size_in_bytes += vertices.size() * sizeof(vertices[0]);
        size_in_bytes += thickness.size() * sizeof(thickness[0]);
        size_in_bytes += color.size() * sizeof(color[0]);
        size_in_bytes += tangents.size() * sizeof(tangents[0]);
        size_in_bytes += indices.size() * sizeof(indices[0]);
        size_in_bytes += sizeof(FileHeader);
        return size_in_bytes;
    }
}
