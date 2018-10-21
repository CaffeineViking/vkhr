#include <vkhr/input_maps.hh>

namespace vkhr {
    InputMapper::InputMapper(Window& window)
                : handle { window.get_handle() } {
        callback_map[handle] = this;
        glfwSetKeyCallback(handle, &key_callback);
        glfwSetMouseButtonCallback(handle, &mouse_button_callback);
    }

    std::unordered_map<GLFWwindow*, InputMapper*> InputMapper::callback_map;

    void InputMapper::mouse_button_callback(GLFWwindow* window, int button,
                                            int action, int) {
        InputMapper* mapper { callback_map[window] };
        Input::MouseButton mouse_button { static_cast<Input::MouseButton>(button) };

        auto mouse_state_iter = mapper->mouse_button_state.find(mouse_button);
        if (mouse_state_iter == mapper->mouse_button_state.end())
            mapper->mouse_button_state[mouse_button] = Input::State::Released;

        Input::State current_state { mapper->mouse_button_state[mouse_button] };
        if (current_state == Input::State::Released && action == GLFW_PRESS)
            mapper->mouse_button_state[mouse_button] = Input::State::JustPressed;
        else
            mapper->mouse_button_state[mouse_button] = static_cast<Input::State>(action);
    }

    void InputMapper::key_callback(GLFWwindow* window, int keyid, int, int action, int) {
        InputMapper* mapper { callback_map[window] };
        Input::Key key { static_cast<Input::Key>(keyid) };

        auto key_state_iter = mapper->key_state_map.find(key);
        if (key_state_iter == mapper->key_state_map.end())
            mapper->key_state_map[key] = Input::State::Released;

        Input::State current_state { mapper->key_state_map[key] };
        if (current_state == Input::State::Released && action == GLFW_PRESS)
            mapper->key_state_map[key] = Input::State::JustPressed;
        else
            mapper->key_state_map[key] = static_cast<Input::State>(action);
    }

    void InputMapper::unbind(const std::string& id) {
        mouse_button_map.erase(id);
        key_map.erase(id);
    }

    void InputMapper::bind(const std::string& id, Input::Key key) {
        key_map.insert({ id, key });
    }

    void InputMapper::bind(const std::string& id, const std::vector<Input::Key>& keys) {
        for (Input::Key key : keys) bind(id, key);
    }

    void InputMapper::bind(const std::string& id, Input::MouseButton mouse_button) {
        mouse_button_map.insert({ id, mouse_button });
    }

    void InputMapper::bind(const std::string& id,
                           const std::vector<Input::MouseButton>& mouse_buttons) {
        for (Input::MouseButton mouse_button : mouse_buttons) bind(id, mouse_button);
    }

    std::vector<Input::MouseButton>
    InputMapper::get_mouse_button_map(const std::string& id) const {
        std::vector<Input::MouseButton> mouse_buttons;
        auto iter_pair = mouse_button_map.equal_range(id);
        for (auto map_iter = iter_pair.first; map_iter != iter_pair.second; ++map_iter)
            mouse_buttons.push_back(map_iter->second);
        return mouse_buttons;
    }

    std::vector<Input::Key> InputMapper::get_key_map(const std::string& id) const {
        std::vector<Input::Key> keys;
        auto iter_pair = key_map.equal_range(id);
        for (auto map_iter = iter_pair.first; map_iter != iter_pair.second; ++map_iter)
            keys.push_back(map_iter->second);
        return keys;
    }

    bool InputMapper::pressed(Input::Key key) const {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end())
            return key_iterator->second == Input::State::Pressed ||
                   key_iterator->second == Input::State::JustPressed ||
                   key_iterator->second == Input::State::Repeat;
        else return false;
    }

    bool InputMapper::just_pressed(Input::Key key) {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end()) {
            if (key_iterator->second == Input::State::JustPressed) {
                key_state_map[key] = Input::State::Pressed;
                return true;
            }
        }

        return false;
    }

    bool InputMapper::released(Input::Key key) const {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end())
            return key_iterator->second == Input::State::Released;
        else return false;
    }

    bool InputMapper::pressed(Input::MouseButton mouse_button) const {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end())
            return mouse_button_iterator->second == Input::State::Pressed ||
                   mouse_button_iterator->second == Input::State::JustPressed ||
                   mouse_button_iterator->second == Input::State::Repeat;
        else return false;
    }

    bool InputMapper::just_pressed(Input::MouseButton mouse_button) {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end()) {
            if (mouse_button_iterator->second == Input::State::JustPressed) {
                mouse_button_state[mouse_button] = Input::State::Pressed;
                return true;
            }
        }

        return false;
    }

    bool InputMapper::released(Input::MouseButton mouse_button) const {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end())
            return mouse_button_iterator->second == Input::State::Released;
        else return false;
    }

    bool InputMapper::pressed(const std::string& id) const {
        auto key_range = key_map.equal_range(id);
        for (auto key_iter = key_range.first; key_iter != key_range.second; ++key_iter)
            if (pressed(key_iter->second)) return true;

        auto mouse_button_range = mouse_button_map.equal_range(id);
        for (auto mouse_button_iter = mouse_button_range.first;
                  mouse_button_iter != mouse_button_range.second;
                  ++mouse_button_iter)
            if (pressed(mouse_button_iter->second)) return true;

        return false;
    }

    bool InputMapper::just_pressed(const std::string& id) {
        auto key_range = key_map.equal_range(id);
        for (auto key_iter = key_range.first; key_iter != key_range.second; ++key_iter)
            if (just_pressed(key_iter->second)) return true;

        auto mouse_button_range = mouse_button_map.equal_range(id);
        for (auto mouse_button_iter = mouse_button_range.first;
                  mouse_button_iter != mouse_button_range.second;
                  ++mouse_button_iter)
            if (just_pressed(mouse_button_iter->second)) return true;

        return false;
    }

    bool InputMapper::released(const std::string& id) const {
        auto key_range = key_map.equal_range(id);
        for (auto key_iter = key_range.first; key_iter != key_range.second; ++key_iter)
            if (released(key_iter->second)) return true;

        auto mouse_button_range = mouse_button_map.equal_range(id);
        for (auto mouse_button_iter = mouse_button_range.first;
                  mouse_button_iter != mouse_button_range.second;
                  ++mouse_button_iter)
            if (released(mouse_button_iter->second)) return true;

        return false;
    }

    glm::vec2 InputMapper::mouse_position() const {
        glm::dvec2 position;
        glfwGetCursorPos(handle, &position.x, &position.y);
        return position;
    }

    void InputMapper::toggle_mouse_lock() {
        mouse_locked = !mouse_locked;
        if (mouse_locked) glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    bool InputMapper::is_mouse_locked() const {
        int mode { glfwGetInputMode(handle, GLFW_CURSOR) };
        if (mode == GLFW_CURSOR_DISABLED) return true;
        else return false;
    }
}
