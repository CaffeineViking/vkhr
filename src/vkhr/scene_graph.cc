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
        }

        if (auto nodes = parser.find("nodes"); nodes != parser.end()) {
            for (auto& node : *nodes) {
            }
        }

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

    const Camera& SceneGraph::get_camera() const {
        return camera;
    }

    Camera& SceneGraph::get_camera() {
        return camera;
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
