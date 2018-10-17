#include <vkhr/hair_style.hh>

#include <cstring>
#include <algorithm>
#include <fstream>

namespace vkhr {
    HairStyle::HairStyle(const std::string& file_path) {
        load(file_path);
    }

    HairStyle::operator bool() const {
        return fail_bit == Error::None;
    }

    HairStyle::Error HairStyle::get_previous_failure_code() const {
        return fail_bit;
    }

    bool HairStyle::load(const std::string& file_path) {
        std::ifstream file { file_path, std::ios::binary };

        if (!file) return fail_bit = Error::OpeningFile, false;

        if (!file.read(reinterpret_cast<char*>(&file_header), sizeof(FileHeader)))
            return fail_bit = Error::ReadingFileHeader, false;

        if (!valid_signature()) return fail_bit = Error::InvalidSignature, false;

        if (!read_segments(file)) return fail_bit = Error::ReadingSegments, false;
        if (!read_vertices(file)) return fail_bit = Error::ReadingVertices, false;
        if (!read_thickness(file)) return fail_bit = Error::ReadingThickness, false;
        if (!read_transparancy(file)) return fail_bit = Error::ReadingTransparency, false;
        if (!read_color(file)) return fail_bit = Error::ReadingColor, false;

        if (!format_is_valid()) return fail_bit = Error::InvalidFormat, false;

        return fail_bit = Error::None, true;
    }

    bool HairStyle::save(const std::string& file_path) const {
        complete_header(); // Fill in remaining header fields.
        if (!format_is_valid()) return fail_bit = Error::InvalidFormat, false;

        std::ofstream file { file_path, std::ios::binary };

        if (!file) return fail_bit = Error::OpeningFile, false;

        if (!file.write(reinterpret_cast<char*>(&file_header), sizeof(FileHeader)))
            return fail_bit = Error::WritingFileHeader, false;

        if (!write_segments(file)) return fail_bit = Error::WritingSegments, false;
        if (!write_vertices(file)) return fail_bit = Error::WritingVertices, false;
        if (!write_thickness(file)) return fail_bit = Error::WritingThickness, false;
        if (!write_transparancy(file)) return fail_bit = Error::WritingTransparency, false;
        if (!write_color(file)) return fail_bit = Error::WritingColor, false;

        return fail_bit = Error::None, true;
    }

    unsigned HairStyle::get_strand_count() const {
        return segments.size();
    }

    unsigned HairStyle::get_vertex_count() const {
        return vertices.size();
    }

    bool HairStyle::has_segments() const { return segments.size(); }
    bool HairStyle::has_vertices() const { return vertices.size(); }
    bool HairStyle::has_thickness() const { return thickness.size(); }
    bool HairStyle::has_transparency() const { return transparency.size(); }
    bool HairStyle::has_color() const { return color.size(); }

    // Below follows boilerplate for writing to the header.

    unsigned HairStyle::get_default_segment_count() const {
        return file_header.default_segment_count;
    }

    void HairStyle::set_default_segment_count(unsigned default_segment_count) {
        file_header.default_segment_count = default_segment_count;
    }

    void HairStyle::set_default_thickness(float default_thickness) {
        file_header.default_thickness = default_thickness;
    }

    float HairStyle::get_default_thickness() const {
        return file_header.default_thickness;
    }

    float HairStyle::get_default_transparency() const {
        return file_header.default_transparency;
    }

    void HairStyle::set_default_transparency(float default_transparency) {
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
        const std::size_t max_size { std::min(sizeof(file_header.information),
                                              information.length() + 1) }; // + terminator.
        std::strncpy(file_header.information, information.c_str(), max_size);
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

        file_header.strand_count = get_strand_count();
        file_header.vertex_count = get_vertex_count();

        update_bitfield();
    }

    void HairStyle::update_bitfield() const {
        file_header.field.has_segments = has_segments();
        file_header.field.has_vertices = has_vertices();
        file_header.field.has_thickness = has_thickness();
        file_header.field.has_transparency = has_transparency();
        file_header.field.has_color = has_color();
        file_header.field.future_extension = 0;
    }

    bool HairStyle::read_segments(std::ifstream& file) {
        if (file_header.field.has_segments) {
            segments.resize(file_header.strand_count);
            if (!file.read(reinterpret_cast<char*>(segments.data()),
                           segments.size() * sizeof(segments[0])))
                return false;
        } return true;
    }

    bool HairStyle::read_vertices(std::ifstream& file) {
        if (file_header.field.has_vertices) {
            vertices.resize(file_header.vertex_count);
            if (!file.read(reinterpret_cast<char*>(vertices.data()),
                           vertices.size() * sizeof(vertices[0])))
                return false;
        } return true;
    }

    bool HairStyle::read_thickness(std::ifstream& file) {
        if (file_header.field.has_thickness) {
            thickness.resize(file_header.vertex_count);
            if (!file.read(reinterpret_cast<char*>(thickness.data()),
                           thickness.size() * sizeof(thickness[0])))
                return false;
        } return true;
    }

    bool HairStyle::read_transparancy(std::ifstream& file) {
        if (file_header.field.has_transparency) {
            transparency.resize(file_header.vertex_count);
            if (!file.read(reinterpret_cast<char*>(transparency.data()),
                           transparency.size() * sizeof(transparency[0])))
                return false;
        } return true;
    }

    bool HairStyle::read_color(std::ifstream& file) {
        if (file_header.field.has_color) {
            color.resize(file_header.vertex_count);
            if (!file.read(reinterpret_cast<char*>(color.data()),
                           color.size() * sizeof(color[0])))
                return false;
        } return true;
    }

    bool HairStyle::write_segments(std::ofstream& file) const {
        if (file_header.field.has_segments) {
            if (!file.write(reinterpret_cast<const char*>(segments.data()),
                            segments.size() * sizeof(segments[0])))
                return false;
        } return true;
    }

    bool HairStyle::write_vertices(std::ofstream& file) const {
        if (file_header.field.has_vertices) {
            if (!file.write(reinterpret_cast<const char*>(vertices.data()),
                            vertices.size() * sizeof(vertices[0])))
                return false;
        } return true;
    }

    bool HairStyle::write_thickness(std::ofstream& file) const {
        if (file_header.field.has_thickness) {
            if (!file.write(reinterpret_cast<const char*>(thickness.data()),
                            thickness.size() * sizeof(thickness[0])))
                return false;
        } return true;
    }

    bool HairStyle::write_transparancy(std::ofstream& file) const {
        if (file_header.field.has_transparency) {
            if (!file.write(reinterpret_cast<const char*>(transparency.data()),
                            transparency.size() * sizeof(transparency[0])))
                return false;
        } return true;
    }

    bool HairStyle::write_color(std::ofstream& file) const {
        if (file_header.field.has_color) {
            if (!file.write(reinterpret_cast<const char*>(color.data()),
                            color.size() * sizeof(color[0])))
                return false;
        } return true;
    }
}
