#ifndef VKHR_SCENE_GRAPH_HH
#define VKHR_SCENE_GRAPH_HH

#include <vkhr/image.hh>
#include <vkhr/model.hh>
#include <vkhr/light_source.hh>
#include <vkhr/hair_style.hh>
#include <vkhr/camera.hh>

#include <glm/glm.hpp>

#include <list>

#include <string>
#include <memory>
#include <vector>

namespace vkhr {
    class SceneGraph final {
    public:
        SceneGraph() = default;
        SceneGraph(const std::string& file_path);

        // Idea: scene graph traversal is done over here, but the renderers
        // specity how they will render each primitive that is found within
        // the scene graph. They provide a rendering function for each, and
        // each function is called: render_thing(thing, transform, camera).
        template<typename RenderModelFunction, typename RenderHairFunction>
        void traverse(RenderModelFunction render_model,
                      RenderHairFunction  render_hair) const;

        bool load(const std::string& file_path);
        bool save(const std::string& file_path) const;

        Model& add(Model&& model);
        Model& add(const Model& model);
        HairStyle& add(const HairStyle& hair_style);
        HairStyle& add(HairStyle&& hair_style);

        void clear();

        bool remove(std::list<Model>::iterator model);
        bool remove(std::list<HairStyle>::iterator hair_style);

        const std::list<HairStyle>& get_hair_styles() const;
        const std::list<Model>& get_models() const;

        const Camera& get_camera() const;
        Camera& get_camera();

        class Node final {
        public:
            void add(Model* model);
            void add(HairStyle* hair_style);
            void reserve_nodes(std::size_t);
            void add(Node* node);

            bool remove(std::vector<Model*>::iterator model);
            bool remove(std::vector<HairStyle*>::iterator hair_style);
            bool remove(std::vector<Node*>::iterator child);

            const std::vector<Node*>& get_children() const;
            const std::vector<HairStyle*>& get_hair_styles() const;
            const std::vector<Model*>& get_models() const;

            void set_rotation(const glm::vec4& rotation);
            void set_translation(const glm::vec3& translation);
            void set_scale(const glm::vec3& scale);

            const glm::vec4& get_rotation() const;
            const glm::vec3& get_translation() const;
            const glm::vec3& get_scale() const;

            const glm::mat4& get_matrix() const;

            void set_node_name(const std::string& n);
            const std::string& get_node_name() const;

        private:
            void recompute_matrix() const;

            glm::vec4 rotation;
            glm::vec3 translation;
            glm::vec3 scale;

            mutable glm::mat4 matrix;
            bool recalculate_matrix { true };

            std::vector<Node*> children;
            std::vector<HairStyle*> hair_styles;
            std::vector<Model*> models;

            std::string node_name;
        };

        const std::vector<Node>& get_nodes() const;

        Node& push_back_node();
        Node& add(const Node& node);
        Node& add(Node&& node);

        Node& get_root_node();

        enum class Error {
            None,

            OpeningFolder,

            ReadingGraphs,

            ReadingCamera,
            ReadingLights,
            ReadingStyles,
            ReadingModels,

            WritingGraphs,

            WritingCamera,
            WritingLights,
            WritingStyles,
            WritingModels
        };

        operator bool() const;
        bool set_error_state(const Error error_state) const;
        Error get_last_error_state() const;

    private:
        Node* root;
        Camera camera;
        std::vector<Node> nodes;
        std::list<LightSource> lights;
        std::list<HairStyle> hair_styles;
        std::list<Model> models;
        mutable Error error_state {
            Error::None
        };
    };

    template<typename RenderModelFunction, typename RenderHairFunction>
    void SceneGraph::traverse(RenderModelFunction render_model,
                              RenderHairFunction  render_hair) const {
        // TODO: do scene graph traversal over here, hopefully.
    }
}

#endif
