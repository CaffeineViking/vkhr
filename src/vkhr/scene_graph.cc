#include <vkhr/scene_graph.hh>

namespace vkhr {
    SceneGraph::SceneGraph(const std::string& file_path) {
        load(file_path);
    }

    bool SceneGraph::load(const std::string& file_path) {
        return true;
    }

    bool SceneGraph::save(const std::string& file_path) const {
        return true;
    }

    void SceneGraph::add(Model&& model) {
        models.emplace_back(std::move(model));
    }

    void SceneGraph::add(const Model& model) {
        models.push_back(model);
    }

    void SceneGraph::add(const HairStyle& hair_style) {
        hair_styles.push_back(hair_style);
    }

    void SceneGraph::add(HairStyle&& hair_style) {
        hair_styles.emplace_back(std::move(hair_style));
    }

    void SceneGraph::clear() {
        models.clear();
        hair_styles.clear();
        root.destroy();
    }

    bool SceneGraph::remove(std::list<Model>::iterator model) {
        return models.erase(model) != models.end();
    }

    bool SceneGraph::remove(std::list<HairStyle>::iterator hair_style) {
        return hair_styles.erase(hair_style) != hair_styles.end();
    }

    SceneGraph::Node& SceneGraph::Node::create_child_node() {
        return children.emplace_back(Node {  });
    }

    void SceneGraph::Node::add(const Node& node) {
        children.push_back(node);
    }

    void SceneGraph::Node::add(Node&& node) {
        children.emplace_back(std::move(node));
    }

    void SceneGraph::Node::add(Model* model) {
        models.push_back(model);
    }

    void SceneGraph::Node::add(HairStyle* hair_style) {
        hair_styles.push_back(hair_style);
    }

    void SceneGraph::Node::destroy() {
        models.clear();
        hair_styles.clear();
        children.clear();
    }

    bool SceneGraph::Node::remove(std::vector<Model*>::iterator model) {
        return models.erase(model) != models.end();
    }

    bool SceneGraph::Node::remove(std::vector<HairStyle*>::iterator hair_style) {
        return hair_styles.erase(hair_style) != hair_styles.end();
    }

    const std::list<HairStyle>& SceneGraph::get_hair_styles() const {
        return hair_styles;
    }

    const std::list<Model>& SceneGraph::get_models() const {
        return models;
    }

    const std::vector<HairStyle*>& SceneGraph::Node::get_hair_styles() const {
        return hair_styles;
    }

    bool SceneGraph::Node::remove(std::vector<Node>::iterator child) {
        return children.erase(child) != children.end();
    }

    const std::vector<SceneGraph::Node>& SceneGraph::Node::get_children() const {
        return children;
    }

    const std::vector<Model*>& SceneGraph::Node::get_models() const {
        return models;
    }

    const glm::mat4& SceneGraph::Node::get_translation() const {
        return translation;
    }

    const glm::mat4& SceneGraph::Node::get_rotation() const {
        return rotation;
    }

    const glm::mat4& SceneGraph::Node::get_scale() const {
        return scale;
    }

    const std::string& SceneGraph::Node::get_node_name() const {
        return node_name;
    }

    SceneGraph::Node& SceneGraph::get_root_node() {
        return root;
    }
}
