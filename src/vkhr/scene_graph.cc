#include <vkhr/scene_graph.hh>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>

namespace vkhr {
    SceneGraph::SceneGraph(const std::string& file_path) {
        load(file_path);
    }

    bool SceneGraph::load(const std::string& file_path) {
        std::ifstream file { file_path };

        if (!file) return set_error_state(Error::OpeningFolder);

        auto parser = json::parse(file);

        scene_path = file_path.substr(0, file_path.find_last_of("\\/") + 1);

        if (auto camera = parser.find("camera"); camera != parser.end()) {
            this->camera.set_field_of_view(camera->value("fieldOfView", 45.0));
            if (auto origin = camera->find("origin"); origin != camera->end()) {
                this->camera.set_position({ origin->at(0),
                                            origin->at(1),
                                            origin->at(2) });
            } else return set_error_state(Error::ReadingCamera);

            if (auto look_at = camera->find("lookAt"); look_at != camera->end()) {
                this->camera.set_look_at_point({ look_at->at(0),
                                                 look_at->at(1),
                                                 look_at->at(2) });
            } else return set_error_state(Error::ReadingCamera);

            if (auto upward = camera->find("upward"); upward != camera->end()) {
                this->camera.set_up_direction({ upward->at(0),
                                                upward->at(1),
                                                upward->at(2) });
            } else this->camera.set_up_direction({ 0, +1.0, 0 });
        } else return set_error_state(Error::ReadingCamera);

        if (auto lights = parser.find("lights"); lights != parser.end()) {
            for (auto& light : *lights) {
                this->lights.push_back(LightSource {/* uninitialized */});

                if (auto position = light.find("position"); position != light.end()) {
                    this->lights.back().set_position({ position->at(0),
                                                       position->at(1),
                                                       position->at(2) });
                } else if (auto direction = light.find("direction"); direction != light.end()) {
                    this->lights.back().set_direction({ direction->at(0),
                                                        direction->at(1),
                                                        direction->at(2) });
                } else return set_error_state(Error::ReadingLights);

                if (auto intensity = light.find("intensity"); intensity != light.end()) {
                    this->lights.back().set_intensity({ intensity->at(0),
                                                        intensity->at(1),
                                                        intensity->at(2) });
                } else return set_error_state(Error::ReadingLights);

                this->lights.back().set_cutoff(light.value("cutoff", 0.0));
            }
        } else return set_error_state(Error::ReadingLights);

        if (auto nodes = parser.find("nodes"); nodes != parser.end()) {
            this->nodes.reserve(nodes->size());
            for (auto& node : *nodes) {
                auto& current_node = push_back_node();

                if (auto scale = node.find("scale"); scale != node.end()) {
                    current_node.set_scale({ scale->at(0),
                                             scale->at(1),
                                             scale->at(2) });
                } else current_node.set_scale({ 1, 1, 1 });

                if (auto rotation = node.find("rotation"); rotation != node.end()) {
                    current_node.set_rotation({ rotation->at(0),
                                                rotation->at(1),
                                                rotation->at(2),
                                                rotation->at(3) });
                } else current_node.set_rotation({ 0, 0, 0, 1 });

                if (auto translation = node.find("translation"); translation != node.end()) {
                    current_node.set_translation({ translation->at(0),
                                                   translation->at(1),
                                                   translation->at(2) });
                } else current_node.set_translation({ 0, 0, 0 });

                current_node.set_node_name(node.value("name", "None"));

                if (auto styles = node.find("styles"); styles != node.end()) {
                    for (auto& style_path : *styles) {
                        if (auto& style = add_hair_style(style_path)) {
                            current_node.add(&style);
                        } else return set_error_state(Error::ReadingStyles);
                    }
                }

                if (auto models = node.find("models"); models != node.end()) {
                    for (auto& model_path : *models) {
                        if (auto& model = add_model(model_path)) {
                            current_node.add(&model);
                        } else return set_error_state(Error::ReadingModels);
                    }
                }
            }
        } else return set_error_state(Error::ReadingGraphs);

        // As a post-process step, link the child nodes together too.
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

        auto root = parser.value("root", 0);
        this->root = &nodes[root];

        return true;
    }

    Model& SceneGraph::add(Model&& model, const std::string& id) {
        models[id] = std::move(model);
        return models[id];
    }

    Model& SceneGraph::add(const Model& model, const std::string& id) {
        models[id] = model;
        return models[id];
    }

    HairStyle& SceneGraph::add(const HairStyle& hair_style, const std::string& id) {
        hair_styles[id] = hair_style;
        return hair_styles[id];
    }

    HairStyle& SceneGraph::add(HairStyle&& hair_style, const std::string& id) {
        hair_styles[id] = std::move(hair_style);
        return hair_styles[id];
    }

    HairStyle& SceneGraph::add_hair_style(const std::string& file_path) {
        auto real_path = scene_path + file_path;

        if (hair_styles.find(real_path) == hair_styles.end())
            hair_styles[real_path] = HairStyle { real_path };

        return hair_styles[real_path];
    }

    Model& SceneGraph::add_model(const std::string& file_path) {
        auto real_path = scene_path + file_path;

        if (models.find(real_path) == models.end())
            models[real_path] = Model { real_path };

        return models[real_path];
    }

    void SceneGraph::clear() {
        models.clear();
        hair_styles.clear();
        nodes.clear();
    }

    bool SceneGraph::remove(ModelMap::iterator model) {
        return models.erase(model) != models.end();
    }

    bool SceneGraph::remove(HairStyleMap::iterator hair_style) {
        return hair_styles.erase(hair_style) != hair_styles.end();
    }

    void SceneGraph::Node::add(Node* node) {
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

    const SceneGraph::HairStyleMap& SceneGraph::get_hair_styles() const {
        return hair_styles;
    }

    const SceneGraph::ModelMap& SceneGraph::get_models() const {
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
    }

    void SceneGraph::Node::set_translation(const glm::vec3& translation) {
        this->translation = translation;
    }

    void SceneGraph::Node::set_scale(const glm::vec3& scale) {
        this->scale = scale;
    }

    const glm::vec3& SceneGraph::Node::get_translation() const {
        return translation;
    }

    const glm::vec4& SceneGraph::Node::get_rotation() const {
        return rotation;
    }

    const glm::vec3& SceneGraph::Node::get_scale() const {
        return scale;
    }

    void SceneGraph::Node::recompute_matrix() const {
    }

    const glm::mat4& SceneGraph::Node::get_matrix() const {
        if (recalculate_matrix)
            recompute_matrix();
        return matrix;
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
        return nodes.back();
    }

    SceneGraph::Node& SceneGraph::add(Node&& node) {
        return nodes.emplace_back(std::move(node));
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
