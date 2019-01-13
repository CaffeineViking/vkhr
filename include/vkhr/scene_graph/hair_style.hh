#ifndef VKHR_HAIR_STYLE_HH
#define VKHR_HAIR_STYLE_HH

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>

#include <glm/gtx/component_wise.hpp>

#include <string>
#include <fstream>
#include <vector>

namespace vkhr {
    struct AABB {
        glm::vec3 origin;
        float radius;
        glm::vec3 size;
        float volume;
    };

    class HairStyle final {
    public:
        HairStyle() = default;
        HairStyle(const std::string& file_path);

        enum class Error {
            None,

            OpeningFile,
            ReadingFileHeader,
            WritingFileHeader,

            InvalidSignature,

            ReadingSegments,
            ReadingVertices,
            ReadingThickness,
            ReadingTransparency,
            ReadingColor,
            ReadingTangents,
            ReadingIndices,

            WritingSegments,
            WritingVertices,
            WritingThickness,
            WritingTransparency,
            WritingColor,
            WritingTangents,
            WritingIndices,

            InvalidFormat
        };

        operator bool() const;
        Error get_last_error_state() const;

        bool load(const std::string& file_path);
        bool save(const std::string& file_path) const;

        unsigned get_strand_count() const;
        void set_strand_count(const unsigned strand_count);
        unsigned get_segment_count() const;
        unsigned get_vertex_count() const;

        bool has_segments() const;
        bool has_vertices() const;
        bool has_thickness() const;
        bool has_transparency() const;
        bool has_color() const;
        bool has_tangents() const;
        bool has_indices() const;
        bool has_bounding_box() const;

        unsigned get_default_segment_count() const;
        void set_default_segment_count(const unsigned default_segment_count);
        void set_default_thickness(const float default_thickness);
        float get_default_thickness() const;
        float get_default_transparency() const;
        void set_default_transparency(const float default_transparency);
        void set_default_color(const glm::vec3& default_color);
        glm::vec3 get_default_color() const;

        void generate_bounding_box();

        AABB get_bounding_box() const;

        struct Volume {
            glm::vec3 resolution;
            AABB bounds;
            std::vector<unsigned char> data;
            bool save(const std::string& file_path);
        };

        Volume voxelize_vertices(std::size_t width, std::size_t height, std::size_t depth) const;
        Volume pregather_density(std::size_t width, std::size_t height, std::size_t depth) const;
        Volume voxelize_segments(std::size_t width, std::size_t height, std::size_t depth) const;

        const char* get_information() const;
        void set_information(const std::string& information);

        void generate_tangents();

        std::vector<glm::vec4> create_position_thickness_data() const;
        std::vector<glm::vec4> create_color_transparency_data() const;

        void generate_indices();

        // Let the user do what he pleases with the hair data.
        // Consistency with arrays is checked upon file write.

        std::vector<unsigned short> segments;
        std::vector<glm::vec3> vertices;
        std::vector<float> thickness;
        std::vector<float> transparency;
        std::vector<glm::vec3> color;

        const std::vector<float>& get_thickness() const;
        const std::vector<glm::vec3>& get_vertices() const;
        const std::vector<unsigned short>& get_segments() const;
        const std::vector<float>& get_transparency() const;
        const std::vector<glm::vec3>& get_color() const;

        std::vector<glm::vec3> tangents;
        std::vector<unsigned>  indices;

        const std::vector<glm::vec3>& get_tangents() const;
        const std::vector<unsigned>&  get_indices()  const;

        std::size_t get_size() const;

    private:
        mutable struct FileHeader {
            char signature[4]; // H, A, I, R.
            unsigned strand_count,
                     vertex_count;

            struct {
                // FIXME: make more portable?!
                unsigned has_segments     : 1,
                         has_vertices     : 1,
                         has_thickness    : 1,
                         has_transparency : 1,
                         has_color        : 1,
                         has_tangents     : 1,
                         has_indices      : 1,
                         has_bounding_box : 1,
                         future_extension : 24;
            } field;

            unsigned default_segment_count;
            float    default_thickness,
                     default_transparency;
            float    default_color[3];

            char information[64];

            float    bounding_box_min[3];
            float    bounding_box_max[3];
        } file_header;

        bool valid_signature() const;
        bool format_is_valid() const;

        void complete_header() const;
        void update_bitfield() const;

        bool set_error_state(const Error error_state) const;

        template<typename T>
        bool read_field(std::ifstream& file, std::vector<T>& field);

        bool read_segments(std::ifstream& file);
        bool read_vertices(std::ifstream& file);
        bool read_thickness(std::ifstream& file);
        bool read_transparancy(std::ifstream& file);
        bool read_color(std::ifstream& file);
        bool read_tangents(std::ifstream& file);
        bool read_indices(std::ifstream& file);

        template<typename T>
        bool write_field(std::ofstream& file, const std::vector<T>& field) const;

        bool write_segments(std::ofstream& file) const;
        bool write_vertices(std::ofstream& file) const;
        bool write_thickness(std::ofstream& file) const;
        bool write_transparancy(std::ofstream& file) const;
        bool write_color(std::ofstream& file) const;
        bool write_tangents(std::ofstream& file) const;
        bool write_indices(std::ofstream& file) const;

        mutable Error error_state { Error::None };
    };

    template<typename T>
    bool HairStyle::read_field(std::ifstream& file, std::vector<T>& field) {
        if (!file.read(reinterpret_cast<char*>(field.data()),
                       field.size() * sizeof(field[0])))
            return false;
        return true;
    }

    template<typename T>
    bool HairStyle::write_field(std::ofstream& file, const std::vector<T>& field) const {
        if (!file.write(reinterpret_cast<const char*>(field.data()),
                        field.size() * sizeof(field[0])))
            return false;
        return true;
    }
}

#endif
