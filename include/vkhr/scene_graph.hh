#ifndef VKHR_SCENE_GRAPH_HH
#define VKHR_SCENE_GRAPH_HH

#include <vkhr/image.hh>
#include <vkhr/model.hh>
#include <vkhr/hair_style.hh>

#include <vkhr/camera.hh>

#include <nlohmann/json.hpp>

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

        void add(Model&& model);
        void add(const Model& model);
        void add(const HairStyle& hair_style);
        void add(HairStyle&& hair_style);

        void clear();

        bool remove(std::list<Model>::iterator model);
        bool remove(std::list<HairStyle>::iterator hair_style);

        const std::list<HairStyle>& get_hair_styles() const;
        const std::list<Model>& get_models() const;

        class Node final {
        public:
            void add(const Node& node);
            void add(Node&& node);
            void add(Model* model);
            void add(HairStyle* hair_style);

            Node& create_child_node();

            void destroy();

            bool remove(std::vector<Model*>::iterator model);
            bool remove(std::vector<HairStyle*>::iterator hair_style);
            bool remove(std::vector<Node>::iterator child);

            const std::vector<Node>& get_children() const;
            const std::vector<HairStyle*>& get_hair_styles() const;
            const std::vector<Model*>& get_models() const;

            // TODO: add functions to do transformations.

            const glm::mat4& get_rotation() const;
            const glm::mat4& get_translation() const;
            const glm::mat4& get_scale() const;

            const std::string& get_node_name() const;

        private:
            glm::mat4 rotation;
            glm::mat4 translation;
            glm::mat4 scale;

            std::vector<Node> children;
            std::vector<HairStyle*> hair_styles;
            std::vector<Model*> models;

            std::string node_name;
        };

        Node& get_root_node();

    private:
        Node root;
        Camera camera;
        std::list<HairStyle> hair_styles;
        std::list<Model> models;
    };

    template<typename RenderModelFunction, typename RenderHairFunction>
    void SceneGraph::traverse(RenderModelFunction render_model,
                              RenderHairFunction  render_hair) const {
        // TODO: do scene graph traversal over here, hopefully.
    }
}

#endif
