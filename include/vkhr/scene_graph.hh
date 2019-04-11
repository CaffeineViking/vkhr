#ifndef VKHR_SCENE_GRAPH_HH
#define VKHR_SCENE_GRAPH_HH

#include <vkhr/scene_graph/model.hh>
#include <vkhr/scene_graph/light_source.hh>
#include <vkhr/scene_graph/hair_style.hh>
#include <vkhr/scene_graph/camera.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <glm/glm.hpp>

#include <list>

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

namespace vkhr {
    class Interface;
    class SceneGraph final {
    public:
        SceneGraph() = default;
        SceneGraph(const std::string& file_path);

        void traverse_nodes();

        bool load(const std::string& scene_path);

        HairStyle& add_style(const std::string& asset_path);
        Model&     add_model(const std::string& asset_path);

        std::string get_uuid();

        Model& add(Model&& model);
        HairStyle& add(HairStyle&& hair_style);
        HairStyle& add(const HairStyle& hair_style);
        Model& add(const Model& model);

        void clear();

        bool remove(std::unordered_map<std::string, Model>::iterator model);
        bool remove(std::unordered_map<std::string, HairStyle>::iterator hair_style);
        const std::unordered_map<std::string, HairStyle>& get_hair_styles() const;
        const std::unordered_map<std::string, Model>& get_models() const;

        mutable std::vector<LightSource::Buffer> light_source_buffers;

        std::vector<LightSource::Buffer>& fetch_light_source_buffers() const;
        const std::list<LightSource>& get_light_sources() const;

        const Camera& get_camera() const;
              Camera& get_camera();
        Camera&   get_new_camera() const;

        const std::string& get_scene_path() const;

        void cleanup();

        class Node final {
        public:
            void add(Model* model);
            void add(HairStyle* hair_style);
            void reserve_nodes(std::size_t);
            void add(Node* node);

            bool remove(std::vector<Model*>::iterator model);
            bool remove(std::vector<HairStyle*>::iterator hair_style);
            bool remove(std::vector<Node*>::iterator child);

            void  set_parent_node(Node* node);
            Node* get_parent_node() const;

            const std::vector<Node*>& get_children() const;
            const std::vector<HairStyle*>& get_hair_styles() const;
            const std::vector<Model*>& get_models() const;

            void scale(const glm::vec3& scale);

            void set_rotation_angle(float angle);
            void set_rotation_axis(const glm::vec3& axis);
            void set_rotation(const glm::vec3& axis, float angle);
            void set_translation(const glm::vec3& translation);
            void set_scale(const glm::vec3& scale);

            float get_rotation_angle() const;
            const glm::vec3& get_rotation_axis() const;
            const glm::vec3& get_translation() const;
            const glm::vec3& get_scale() const;

            const glm::mat4& get_local_transform() const;

            void set_model_matrix(const glm::mat4& m);
            const glm::mat4& get_model_matrix() const;

            const glm::mat4& get_matrix() const;

            void set_node_name(const std::string& n);
            const std::string& get_node_name() const;

        private:
            void recompute_transform() const;

            float rotation_angle;
            glm::vec3 rotation_axis;
            glm::vec3 translation;
            glm::vec3 scaling;

            mutable glm::mat4 transform;

            mutable bool recalculate_transform { true };

            mutable glm::mat4 model_matrix;

            std::vector<Node*> children;
            std::vector<HairStyle*> hair_styles;
            std::vector<Model*> models;

            Node* parent { nullptr };

            std::string node_name;

            friend class Interface;
        };

        Node* find_node_by_name(const std::string& name);

        std::size_t get_strand_count() const;
        std::size_t get_memory_usage() const;

        const std::vector<Node*>& get_nodes_with_models() const;
        const std::vector<Node*>& get_nodes_with_hair_styles() const;

        const std::unordered_map<std::string, Node*>& get_named_nodes() const;

        const std::vector<Node>& get_nodes() const;

        Node& push_back_node();
        Node& add(const Node& node);
        Node& add(Node&& node);

        bool remove(std::vector<Node>::iterator node);

        unsigned get_root_index();

        Node& get_root_node();

        enum class Error {
            None,

            OpeningFolder,

            ReadingCamera,
            ReadingLight,
            ReadingNode,
            ReadingStyle,
            ReadingModel,
        };

        operator bool() const;
        bool set_error_state(const Error error_state) const;
        Error get_last_error_state() const;

    private:
        void traverse(Node& node, const glm::mat4& parent_matrix);

        void link_nodes(nlohmann::json& parser);

        bool parse_camera(nlohmann::json& parser, Camera& camera);
        bool parse_light(nlohmann::json& parser,  LightSource& light);
        bool parse_node(nlohmann::json& parser,   Node& node, int i);

        void build_node_cache(Node& chnode);
        void destroy_previous_node_caches();

        void rebuild_lights_buffer_caches();

        std::vector<Node*> hair_style_cache;
        std::vector<Node*> model_node_cache;

        Node* root;
        unsigned root_index = 0;
        mutable Camera camera;
        std::vector<Node> nodes;
        std::list<LightSource> light_sources;

        std::unordered_map<std::string, Node*> nodes_by_name;
        std::unordered_map<std::string, HairStyle> hair_styles;
        std::unordered_map<std::string, Model> models;

        std::size_t unique_name { 0 };
        std::string scene_path { "" };

        mutable Error error_state {
            Error::None
        };

        friend class vkhr::Interface;
    };
}

#endif
