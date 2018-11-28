#include <vkhr/scene_graph.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>

#include <stdexcept>

namespace vkhr {
    SceneGraph::SceneGraph(const std::string& file_path) {
        load(file_path);
    }

    void SceneGraph::traverse_nodes() {
        destroy_previous_node_caches();
        const glm::mat4 identity { 1 };
        traverse(*root, identity); // I
    }

    void SceneGraph::traverse(Node& node, const glm::mat4& parent_matrix) {
        node.set_model_matrix(node.get_local_transform() * parent_matrix);

        auto& model_matrix = node.get_model_matrix();

        build_node_cache(node);

        for (auto& child_node : node.get_children())
            traverse(*child_node, model_matrix);
    }

    bool SceneGraph::load(const std::string& file_path) {
        std::ifstream file { file_path };

        if (!file) return set_error_state(Error::OpeningFolder);

        auto parser = json::parse(file);

        scene_path = file_path.substr(0, file_path.find_last_of("\\/") + 1);

        if (!parse_camera(parser, camera)) return set_error_state(Error::ReadingCamera);

        if (auto lights = parser.find("lights"); lights != parser.end()) {
            for (auto& light : *lights) {
                this->lights.emplace_back(vkhr::LightSource{});
                if (!parse_light(light, this->lights.back())) {
                    return set_error_state(Error::ReadingLight);
                }
            }
        }

        if (auto nodes = parser.find("nodes"); nodes != parser.end()) {
            this->nodes.reserve(nodes->size());
            for (auto& node : *nodes) {
                auto& current_node = push_back_node();
                if (!parse_node(node, current_node)) {
                    return set_error_state(Error::ReadingNode);
                }
            }
        }

        link_nodes(parser);

        auto root = parser.value("root", 0);
        this->root = &nodes[root];

        return true;
    }

    bool SceneGraph::parse_camera(nlohmann::json& parser, Camera& scene_camera) {
        if (auto camera = parser.find("camera"); camera != parser.end()) {
            scene_camera.set_field_of_view(glm::radians(camera->value("fieldOfView", 45.0)));
            if (auto origin = camera->find("origin"); origin != camera->end()) {
                scene_camera.set_position({ origin->at(0),
                                            origin->at(1),
                                            origin->at(2) });
            } else return set_error_state(Error::ReadingCamera);

            if (auto look_at = camera->find("lookAt"); look_at != camera->end()) {
                scene_camera.set_look_at_point({ look_at->at(0),
                                                 look_at->at(1),
                                                 look_at->at(2) });
            } else return set_error_state(Error::ReadingCamera);

            if (auto upward = camera->find("upward"); upward != camera->end()) {
                scene_camera.set_up_direction({ upward->at(0),
                                                upward->at(1),
                                                upward->at(2) });
            } else scene_camera.set_up_direction({ 0, +1.0, 0 });
        } else return set_error_state(Error::ReadingCamera);

        return true;
    }

    bool SceneGraph::parse_light(nlohmann::json& parser, LightSource& light) {
        if (auto position = parser.find("position"); position != parser.end()) {
            light.set_position({ position->at(0),
                                 position->at(1),
                                 position->at(2) });
        } else if (auto direction = parser.find("direction"); direction != parser.end()) {
            light.set_direction({ direction->at(0),
                                  direction->at(1),
                                  direction->at(2) });
        } else return set_error_state(Error::ReadingLight);

        if (auto intensity = parser.find("intensity"); intensity != parser.end()) {
            light.set_intensity({ intensity->at(0),
                                  intensity->at(1),
                                  intensity->at(2) });
        } else return set_error_state(Error::ReadingLight);

        light.set_cutoff(parser.value("cutoff", 0.0));

        return true;
    }

    bool SceneGraph::parse_node(nlohmann::json& parser, Node& node) {
        if (auto scale = parser.find("scale"); scale != parser.end()) {
            node.set_scale({ scale->at(0),
                             scale->at(1),
                             scale->at(2) });
        } else node.set_scale({ 1, 1, 1 });

        if (auto rotate = parser.find("rotate"); rotate != parser.end()) {
            node.set_rotation({ rotate->at(0),
                                rotate->at(1),
                                rotate->at(2),
                                rotate->at(3) });
        } else node.set_rotation({ 1, 0, 0, 0 });

        if (auto translate = parser.find("translate"); translate != parser.end()) {
            node.set_translation({ translate->at(0),
                                   translate->at(1),
                                   translate->at(2) });
        } else node.set_translation({ 0, 0, 0 });

        node.set_node_name(parser.value("name", "None"));
        nodes_by_name[node.get_node_name()] = &node;

        if (auto styles = parser.find("styles"); styles != parser.end()) {
            for (auto style_path : *styles) {
                if (auto& style = add_style(style_path))
                    node.add(&style);
                else return set_error_state(Error::ReadingStyle);
            }
        }

        if (auto models = parser.find("models"); models != parser.end()) {
            for (auto& model_path : *models) {
                if (auto& model = add_model(model_path))
                    node.add(&model);
                else return set_error_state(Error::ReadingModel);
            }
        }

        return true;
    }

    void SceneGraph::link_nodes(nlohmann::json& parser) {
        if (auto nodes = parser.find("nodes"); nodes != parser.end()) {
            std::size_t node_id { 0 };
            for (auto& node : *nodes) {
                auto& current_node = this->nodes[node_id++];
                if (auto children = node.find("children"); children != node.end()) {
                    current_node.reserve_nodes(children->size());
                    for (auto& child : *children)
                        current_node.add(&this->nodes[child]);
                }
            }
        }
    }

    std::string SceneGraph::get_uuid() {
        return std::to_string(unique_name++);
    }

    Model& SceneGraph::add(Model&& model) {
        auto name = get_uuid();
        models[name] = std::move(model);
        return models[name];
    }

    Model& SceneGraph::add(const Model& model) {
        auto name = get_uuid();
        models[name] = model;
        return models[name];
    }

    HairStyle& SceneGraph::add(const HairStyle& hair_style) {
        auto name = get_uuid();
        hair_styles[name] = hair_style;
        return hair_styles[name];
    }

    HairStyle& SceneGraph::add(HairStyle&& hair_style) {
        auto name = get_uuid();
        hair_styles[name] = std::move(hair_style);
        return hair_styles[name];
    }

    HairStyle& SceneGraph::add_style(const std::string& asset_path) {
        auto path = scene_path + asset_path;
        hair_styles[path] = HairStyle { path };

        // If you get this exception, it most likely means you haven't cloned using Git LFS.
        if (!hair_styles[path]) throw std::runtime_error { "Couldn't find: " + path + "!" };

        if (!hair_styles[path].has_tangents())
            hair_styles[path].generate_tangents();
        hair_styles[path].set_default_thickness(0.14f);
        if (!hair_styles[path].has_indices())
            hair_styles[path].generate_indices();
        return hair_styles[path];
    }

    Model& SceneGraph::add_model(const std::string& asset_path) {
        auto path = scene_path + asset_path;
        models[path] = Model { path };

        // Same thing here, this is likely the failure of not having Git LFS installed.
        if (!models[path]) throw std::runtime_error { "Couldn't find: " + path + "!" };

        return models[path];
    }

    void SceneGraph::clear() {
        models.clear();
        hair_styles.clear();
        nodes.clear();
        nodes_by_name.clear();
    }

    bool SceneGraph::remove(std::unordered_map<std::string, Model>::iterator model) {
        return models.erase(model) != models.end();
    }

    bool SceneGraph::remove(std::unordered_map<std::string, HairStyle>::iterator hair_style) {
        return hair_styles.erase(hair_style) != hair_styles.end();
    }

    void SceneGraph::Node::add(Node* node) {
        node->set_parent_node(this);
        children.push_back(node);
    }

    void SceneGraph::Node::add(Model* model) {
        models.push_back(model);
    }

    void SceneGraph::Node::add(HairStyle* hair_style) {
        hair_styles.push_back(hair_style);
    }

    bool SceneGraph::Node::remove(std::vector<Model*>::iterator model) {
        return models.erase(model) != models.end();
    }

    bool SceneGraph::Node::remove(std::vector<HairStyle*>::iterator hair_style) {
        return hair_styles.erase(hair_style) != hair_styles.end();
    }

    const std::unordered_map<std::string, HairStyle>& SceneGraph::get_hair_styles() const {
        return hair_styles;
    }

    const std::unordered_map<std::string, Model>& SceneGraph::get_models() const {
        return models;
    }

    const Camera& SceneGraph::get_camera() const {
        return camera;
    }

    Camera& SceneGraph::get_camera() {
        return camera;
    }

    const std::vector<HairStyle*>& SceneGraph::Node::get_hair_styles() const {
        return hair_styles;
    }

    bool SceneGraph::Node::remove(std::vector<Node*>::iterator child) {
        return children.erase(child) != children.end();
    }

    const std::vector<SceneGraph::Node*>& SceneGraph::Node::get_children() const {
        return children;
    }

    const std::vector<Model*>& SceneGraph::Node::get_models() const {
        return models;
    }

    void SceneGraph::Node::set_rotation(const glm::vec4& rotation) {
        this->rotation = rotation;
        recalculate_transform = true;
    }

    void SceneGraph::Node::set_translation(const glm::vec3& translation) {
        this->translation = translation;
        recalculate_transform = true;
    }

    void SceneGraph::Node::set_scale(const glm::vec3& scale) {
        this->scaling = scale;
        recalculate_transform = true;
    }

    const glm::vec3& SceneGraph::Node::get_translation() const {
        return translation;
    }

    const glm::vec4& SceneGraph::Node::get_rotation() const {
        return rotation;
    }

    const glm::vec3& SceneGraph::Node::get_scale() const {
        return scaling;
    }

    void SceneGraph::Node::set_parent_node(Node* node) {
        parent = node;
    }

    SceneGraph::Node* SceneGraph::Node::get_parent_node() const {
        return parent;
    }

    void SceneGraph::Node::recompute_transform() const {
        transform = glm::mat4 { 1.0 };
        transform = glm::translate(transform, translation);
        auto axis = glm::vec3 { rotation };
        transform = glm::rotate(transform, rotation.w, axis);
        transform = glm::scale(transform, scaling);
        recalculate_transform = false;
    }

    const glm::mat4& SceneGraph::Node::get_local_transform() const {
        if (recalculate_transform)
            recompute_transform();
        return transform;
    }

    void SceneGraph::Node::set_model_matrix(const glm::mat4& matrix) {
        model_matrix = matrix;
    }

    const glm::mat4& SceneGraph::Node::get_model_matrix() const {
        return model_matrix;
    }

    const glm::mat4& SceneGraph::Node::get_m() const {
        return model_matrix;
    }

    void SceneGraph::Node::set_node_name(const std::string& name) {
        node_name = name;
    }

    void SceneGraph::Node::reserve_nodes(std::size_t count) {
        children.reserve(count);
    }

    const std::string& SceneGraph::Node::get_node_name() const {
        return node_name;
    }

    const std::vector<SceneGraph::Node>& SceneGraph::get_nodes() const {
        return nodes;
    }

    SceneGraph::Node& SceneGraph::add(const Node& node) {
        nodes.push_back(node);
        nodes_by_name[node.get_node_name()] = &nodes.back();
        return nodes.back();
    }

    SceneGraph::Node& SceneGraph::add(Node&& node) {
        nodes.emplace_back(std::move(node));
        nodes_by_name[node.get_node_name()] = &nodes.back();
        return nodes.back();
    }

    bool SceneGraph::remove(std::vector<Node>::iterator node) {
        if (node != nodes.end()) {
            auto node_name = node->get_node_name();
            if (node_name != "None")
                nodes_by_name.erase(nodes_by_name.find(node_name));
        }

        return nodes.erase(node) != nodes.end();
    }

    SceneGraph::Node* SceneGraph::find_node_by_name(const std::string& name) {
        if (nodes_by_name.find(name) != nodes_by_name.end())
            return nodes_by_name[name];
        else
            return nullptr;
    }

    const std::vector<SceneGraph::Node*>& SceneGraph::get_nodes_with_models() const {
        return model_node_cache;
    }

    const std::vector<SceneGraph::Node*>& SceneGraph::get_nodes_with_hair_styles() const {
        return hair_style_cache;
    }

    const std::unordered_map<std::string, SceneGraph::Node*>& SceneGraph::get_named_nodes() const {
        return nodes_by_name;
    }
    void SceneGraph::build_node_cache(Node& node) {
        for (std::size_t i { }; i < node.get_models().size(); ++i)
            model_node_cache.push_back(&node);
        for (std::size_t i { }; i < node.get_hair_styles().size(); ++i)
            hair_style_cache.push_back(&node);
    }

    void SceneGraph::destroy_previous_node_caches() {
        model_node_cache.clear();
        hair_style_cache.clear();
    }

    SceneGraph::Node& SceneGraph::push_back_node() {
        return nodes.emplace_back();
    }

    SceneGraph::Node& SceneGraph::get_root_node() {
        return *root;
    }

    SceneGraph::operator bool() const {
        return error_state == Error::None;
    }

    bool SceneGraph::set_error_state(const Error error_state) const {
        this->error_state = error_state;
        if (error_state == Error::None) {
            return true;
        } else return false;
    }

    SceneGraph::Error SceneGraph::get_last_error_state() const {
        return error_state;
    }
}
