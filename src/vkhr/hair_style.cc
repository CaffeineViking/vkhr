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

    std::string HairStyle::get_information() const {
        return file_header.information;
    }

    void HairStyle::set_information(const std::string& information) {
        const std::size_t info_size { sizeof(file_header.information) };
        std::memset(file_header.information, '\0', info_size); // For consistency.
        const std::size_t copy_size { std::min(info_size, information.length()) };
        std::strncpy(file_header.information, information.c_str(), copy_size);
    }

    void HairStyle::generate_tangents() {
        std::size_t vertex { 0 };
        tangents.reserve(get_vertex_count());
        for (std::size_t strand { 0 }; strand < get_strand_count(); ++strand) {
            unsigned segment_count { get_default_segment_count() };
            if (has_segments()) {
                segment_count = segments[strand];
            }

            // Special tangent (at the start of an segment).
            tangents.push_back(glm::vec3 { 0.0, 0.0, 0.0 });

            ++vertex; // That first one is a vertex in this.

            for (std::size_t segment { 1 }; segment < segment_count; ++segment) {
                const auto& current_vertex { vertices[vertex + 0] };
                const auto& next_vertex    { vertices[vertex + 1] };
                const auto tangent { next_vertex - current_vertex };

                tangents.push_back(glm::normalize(tangent));
                ++vertex;
            }

            // Special tangent (at the end of the segments).
            tangents.push_back(glm::vec3 { 0.0, 0.0, 0.0 });

            ++vertex; // The last segment is also an vertex.
        }
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
        if (has_tangents() && tangents.size() != vertices.size()) return false;
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
}
