#include <vkhr/input_map.hh>

namespace vkhr {
    InputMap::InputMap(Window& window)
        : handle { window.get_handle() } {
        callback_map[handle] = this;
        glfwSetKeyCallback(handle, &key_callback);
        glfwSetMouseButtonCallback(handle, &mouse_button_callback);
        glfwSetScrollCallback(handle, &scroll_callback);
        glfwSetCharCallback(handle, &char_callback);
    }

    std::unordered_map<GLFWwindow*, InputMap*> InputMap::callback_map;

    void InputMap::mouse_button_callback(GLFWwindow* window, int button,
                                            int action, int mods) {
        InputMap* mapper { callback_map[window] };
        Input::MouseButton mouse_button { static_cast<Input::MouseButton>(button) };

        auto mouse_state_iter = mapper->mouse_button_state.find(mouse_button);
        if (mouse_state_iter == mapper->mouse_button_state.end())
            mapper->mouse_button_state[mouse_button] = Input::State::Released;

        Input::State current_state { mapper->mouse_button_state[mouse_button] };
        if ((current_state == Input::State::Released ||
             current_state == Input::State::JustReleased) &&
            action == GLFW_PRESS)
            mapper->mouse_button_state[mouse_button] = Input::State::JustPressed;
        else if ((current_state == Input::State::Pressed ||
                  current_state == Input::State::Repeat ||
                  current_state == Input::State::JustPressed) &&
                 action == GLFW_RELEASE)
            mapper->mouse_button_state[mouse_button] = Input::State::JustReleased;
        else
            mapper->mouse_button_state[mouse_button] = static_cast<Input::State>(action);

        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    }

    void InputMap::key_callback(GLFWwindow* window, int keyid, int scanid, int action, int mods) {
        InputMap* mapper { callback_map[window] };
        Input::Key key { static_cast<Input::Key>(keyid) };

        auto key_state_iter = mapper->key_state_map.find(key);
        if (key_state_iter == mapper->key_state_map.end())
            mapper->key_state_map[key] = Input::State::Released;

        Input::State current_state { mapper->key_state_map[key] };
        if ((current_state == Input::State::Released ||
             current_state == Input::State::JustReleased) &&
            action == GLFW_PRESS)
            mapper->key_state_map[key] = Input::State::JustPressed;
        else if ((current_state == Input::State::Pressed ||
                  current_state == Input::State::Repeat ||
                  current_state == Input::State::JustPressed) &&
                 action == GLFW_RELEASE)
            mapper->key_state_map[key] = Input::State::JustReleased;
        else
            mapper->key_state_map[key] = static_cast<Input::State>(action);

        ImGui_ImplGlfw_KeyCallback(window, keyid, scanid, action, mods);
    }

    void InputMap::scroll_callback(GLFWwindow* window, double scroll_x, double scroll_y) {
        InputMap* mapper { callback_map[window] };
        mapper->scroll_offsets.x = scroll_x;
        mapper->scroll_offsets.y = scroll_y;
        ImGui_ImplGlfw_ScrollCallback(window, scroll_x, scroll_y);
    }

    void InputMap::char_callback(GLFWwindow* window, unsigned int codepoint) {
        ImGui_ImplGlfw_CharCallback(window, codepoint);
    }

    void InputMap::unbind(const std::string& id) {
        mouse_button_map.erase(id);
        key_map.erase(id);
    }

    void InputMap::bind(const std::string& id, Input::Key key) {
        key_map.insert({ id, key });
    }

    void InputMap::bind(const std::string& id, const std::vector<Input::Key>& keys) {
        for (Input::Key key : keys) bind(id, key);
    }

    void InputMap::bind(const std::string& id, Input::MouseButton mouse_button) {
        mouse_button_map.insert({ id, mouse_button });
    }

    void InputMap::bind(const std::string& id,
                           const std::vector<Input::MouseButton>& mouse_buttons) {
        for (Input::MouseButton mouse_button : mouse_buttons) bind(id, mouse_button);
    }

    std::vector<Input::MouseButton>
    InputMap::get_mouse_button_map(const std::string& id) const {
        std::vector<Input::MouseButton> mouse_buttons;
        auto iter_pair = mouse_button_map.equal_range(id);
        for (auto map_iter = iter_pair.first; map_iter != iter_pair.second; ++map_iter)
            mouse_buttons.push_back(map_iter->second);
        return mouse_buttons;
    }

    std::vector<Input::Key> InputMap::get_key_map(const std::string& id) const {
        std::vector<Input::Key> keys;
        auto iter_pair = key_map.equal_range(id);
        for (auto map_iter = iter_pair.first; map_iter != iter_pair.second; ++map_iter)
            keys.push_back(map_iter->second);
        return keys;
    }

    bool InputMap::pressed(Input::Key key) const {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end())
            return key_iterator->second == Input::State::Pressed ||
                   key_iterator->second == Input::State::JustPressed ||
                   key_iterator->second == Input::State::Repeat;
        else return false;
    }

    bool InputMap::just_pressed(Input::Key key) {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end()) {
            if (key_iterator->second == Input::State::JustPressed) {
                key_state_map[key] = Input::State::Pressed;
                return true;
            }
        }

        return false;
    }

    bool InputMap::just_released(Input::Key key) {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end()) {
            if (key_iterator->second == Input::State::JustReleased) {
                key_state_map[key] = Input::State::Released;
                return true;
            }
        }

        return false;
    }

    bool InputMap::released(Input::Key key) const {
        auto key_iterator = key_state_map.find(key);
        if (key_iterator != key_state_map.end())
            return key_iterator->second == Input::State::Released ||
                   key_iterator->second == Input::State::JustReleased;
        else return false;
    }

    bool InputMap::pressed(Input::MouseButton mouse_button) const {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end())
            return mouse_button_iterator->second == Input::State::Pressed ||
                   mouse_button_iterator->second == Input::State::JustPressed ||
                   mouse_button_iterator->second == Input::State::Repeat;
        else return false;
    }

    bool InputMap::just_pressed(Input::MouseButton mouse_button) {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end()) {
            if (mouse_button_iterator->second == Input::State::JustPressed) {
                mouse_button_state[mouse_button] = Input::State::Pressed;
                return true;
            }
        }

        return false;
    }

    bool InputMap::just_released(Input::MouseButton mouse_button) {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end()) {
            if (mouse_button_iterator->second == Input::State::JustReleased) {
                mouse_button_state[mouse_button] = Input::State::Released;
                return true;
            }
        }

        return false;
    }

    bool InputMap::released(Input::MouseButton mouse_button) const {
        auto mouse_button_iterator = mouse_button_state.find(mouse_button);
        if (mouse_button_iterator != mouse_button_state.end())
            return mouse_button_iterator->second == Input::State::Released ||
                   mouse_button_iterator->second == Input::State::JustReleased;
        else return false;
    }

    bool InputMap::pressed(const std::string& id) const {
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

    bool InputMap::just_pressed(const std::string& id) {
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

    bool InputMap::just_released(const std::string& id) {
        auto key_range = key_map.equal_range(id);
        for (auto key_iter = key_range.first; key_iter != key_range.second; ++key_iter)
            if (just_released(key_iter->second)) return true;

        auto mouse_button_range = mouse_button_map.equal_range(id);
        for (auto mouse_button_iter = mouse_button_range.first;
                  mouse_button_iter != mouse_button_range.second;
                  ++mouse_button_iter)
            if (just_released(mouse_button_iter->second)) return true;

        return false;
    }

    bool InputMap::released(const std::string& id) const {
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

    glm::vec2 InputMap::get_mouse_position() const {
        glm::dvec2 position;
        glfwGetCursorPos(handle, &position.x, &position.y);
        return position;
    }

    glm::vec2 InputMap::get_scroll_offset() const {
        return scroll_offsets;
    }

    void InputMap::reset_scrolling_offset() {
        scroll_offsets = { 0.0f, 0.0f };
    }

    void InputMap::freeze_cursor() {
        if (!is_mouse_locked())
            toggle_mouse_lock();
    }

    void InputMap::unlock_cursor() {
        if (is_mouse_locked())
            toggle_mouse_lock();
    }

    void InputMap::toggle_mouse_lock() {
        mouse_locked = !mouse_locked;
        if (mouse_locked) glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    bool InputMap::is_mouse_locked() const {
        int mode { glfwGetInputMode(handle, GLFW_CURSOR) };
        if (mode == GLFW_CURSOR_DISABLED) return true;
        else return false;
    }
}
