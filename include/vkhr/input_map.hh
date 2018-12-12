#ifndef VKHR_INPUT_MAP_HH
#define VKHR_INPUT_MAP_HH

#include <vkhr/window.hh>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>

namespace vkhr {
    namespace Input {
        enum class State {
            Pressed = GLFW_PRESS,
            Released = GLFW_RELEASE,
            Repeat = GLFW_REPEAT,
            JustPressed,
            JustReleased
        };

        enum class Key {
            Unknown = GLFW_KEY_UNKNOWN, Space = GLFW_KEY_SPACE,
            Apostrophe = GLFW_KEY_APOSTROPHE, Comma = GLFW_KEY_COMMA,
            Minus = GLFW_KEY_MINUS, Period = GLFW_KEY_PERIOD,
            Slash = GLFW_KEY_SLASH,

            Zero = GLFW_KEY_0, One = GLFW_KEY_1, Two = GLFW_KEY_2,
            Three = GLFW_KEY_3, Four = GLFW_KEY_4, Five = GLFW_KEY_5,
            Six = GLFW_KEY_6, Seven = GLFW_KEY_7, Eight = GLFW_KEY_8,

            Nine = GLFW_KEY_9, Semicolon = GLFW_KEY_SEMICOLON,
            Equal = GLFW_KEY_EQUAL,

            A = GLFW_KEY_A, B = GLFW_KEY_B, C = GLFW_KEY_C,
            D = GLFW_KEY_D, E = GLFW_KEY_E, F = GLFW_KEY_F,
            G = GLFW_KEY_G, H = GLFW_KEY_H, I = GLFW_KEY_I,
            J = GLFW_KEY_J, K = GLFW_KEY_K, L = GLFW_KEY_L,
            M = GLFW_KEY_M, N = GLFW_KEY_N, O = GLFW_KEY_O,
            P = GLFW_KEY_P, Q = GLFW_KEY_Q, R = GLFW_KEY_R,
            S = GLFW_KEY_S, T = GLFW_KEY_T, U = GLFW_KEY_U,
            V = GLFW_KEY_V, W = GLFW_KEY_W, X = GLFW_KEY_X,
            Y = GLFW_KEY_Y, Z = GLFW_KEY_Z,

            LeftBracket = GLFW_KEY_LEFT_BRACKET,
            Backslash = GLFW_KEY_BACKSLASH,
            RightBracket = GLFW_KEY_RIGHT_BRACKET,
            GraveAccent = GLFW_KEY_GRAVE_ACCENT,
            World1 = GLFW_KEY_WORLD_1,
            World2 = GLFW_KEY_WORLD_2,

            Escape = GLFW_KEY_ESCAPE,
            Enter = GLFW_KEY_ENTER,
            Tab = GLFW_KEY_TAB,
            Backspace  = GLFW_KEY_BACKSPACE,
            Insert = GLFW_KEY_INSERT,
            Delete = GLFW_KEY_DELETE,

            Right = GLFW_KEY_RIGHT,
            Left = GLFW_KEY_LEFT,
            Down = GLFW_KEY_DOWN,
            Up = GLFW_KEY_UP,

            PageUp = GLFW_KEY_PAGE_UP,
            PageDown = GLFW_KEY_PAGE_DOWN,
            Home = GLFW_KEY_HOME,
            End = GLFW_KEY_END,
            CapsLock = GLFW_KEY_CAPS_LOCK,
            ScrollLock = GLFW_KEY_SCROLL_LOCK,
            NumLock = GLFW_KEY_NUM_LOCK,
            PrintScreen = GLFW_KEY_PRINT_SCREEN,
            Pause = GLFW_KEY_PAUSE,

            F1  = GLFW_KEY_F1, F2  = GLFW_KEY_F2,
            F3  = GLFW_KEY_F3, F4  = GLFW_KEY_F4,
            F5  = GLFW_KEY_F5, F6  = GLFW_KEY_F6,
            F7  = GLFW_KEY_F7, F8  = GLFW_KEY_F8,
            F9  = GLFW_KEY_F9, F10 = GLFW_KEY_F10,
            F11 = GLFW_KEY_F11, F12 = GLFW_KEY_F12,
            F13 = GLFW_KEY_F13, F14 = GLFW_KEY_F14,
            F15 = GLFW_KEY_F15, F16 = GLFW_KEY_F16,
            F17 = GLFW_KEY_F17, F18 = GLFW_KEY_F18,
            F19 = GLFW_KEY_F19, F20 = GLFW_KEY_F20,
            F21 = GLFW_KEY_F21, F22 = GLFW_KEY_F22,
            F23 = GLFW_KEY_F23, F24 = GLFW_KEY_F24,
            F25 = GLFW_KEY_F25,

            Keypad0 = GLFW_KEY_KP_0, Keypad1 = GLFW_KEY_KP_1,
            Keypad2 = GLFW_KEY_KP_2, Keypad3 = GLFW_KEY_KP_3,
            Keypad4 = GLFW_KEY_KP_4, Keypad5 = GLFW_KEY_KP_5,
            Keypad6 = GLFW_KEY_KP_6, Keypad7 = GLFW_KEY_KP_7,
            Keypad8 = GLFW_KEY_KP_8, Keypad9 = GLFW_KEY_KP_9,

            KeypadDecimal = GLFW_KEY_KP_DECIMAL,
            KeypadDivide = GLFW_KEY_KP_DIVIDE,
            KeypadMultiply = GLFW_KEY_KP_MULTIPLY,
            KeypadSubtract = GLFW_KEY_KP_SUBTRACT,
            KeypadAdd = GLFW_KEY_KP_ADD,
            KeypadEnter = GLFW_KEY_KP_ENTER,
            KeypadEqual = GLFW_KEY_KP_EQUAL,

            LeftShift = GLFW_KEY_LEFT_SHIFT,
            LeftControl = GLFW_KEY_LEFT_CONTROL,
            LeftAlt = GLFW_KEY_LEFT_ALT,
            LeftSuper = GLFW_KEY_LEFT_SUPER,
            RightShift = GLFW_KEY_RIGHT_SHIFT,
            RightControl = GLFW_KEY_RIGHT_CONTROL,
            RightAlt = GLFW_KEY_RIGHT_ALT,
            RightSuper = GLFW_KEY_RIGHT_SUPER,
            Menu = GLFW_KEY_MENU
        };

        enum class MouseButton {
            Left = GLFW_MOUSE_BUTTON_LEFT,
            Middle = GLFW_MOUSE_BUTTON_MIDDLE,
            Right = GLFW_MOUSE_BUTTON_RIGHT
        };
    }

    class InputMap final {
    public:
        InputMap(Window& window);

        void unbind(const std::string& id);

        void bind(const std::string& id, Input::Key key);
        void bind(const std::string& id, const std::vector<Input::Key>& keys);
        void bind(const std::string& id, Input::MouseButton mouse_button);
        void bind(const std::string& id,
                  const std::vector<Input::MouseButton>& mouse_buttons);

        std::vector<Input::MouseButton> get_mouse_button_map(const std::string& id) const;
        std::vector<Input::Key> get_key_map(const std::string& id) const;

        bool pressed(Input::Key key) const;
        bool just_pressed(Input::Key key);
        bool just_released(Input::Key key);
        bool released(Input::Key key) const;
        bool pressed(Input::MouseButton key) const;
        bool just_pressed(Input::MouseButton key);
        bool just_released(Input::MouseButton key);
        bool released(Input::MouseButton key) const;

        bool pressed(const std::string& id) const;
        bool just_pressed(const std::string& id);
        bool just_released(const std::string& id);
        bool released(const std::string& id) const;

        glm::vec2 get_mouse_position() const;
        glm::vec2 get_scroll_offset() const;
        void reset_scrolling_offset();

        void freeze_cursor();
        void unlock_cursor();

        void toggle_mouse_lock();
        bool is_mouse_locked() const;

    private:
        // FIXME: Since we can't pass around private member functions
        // to the GLFW callback functions, we do a hack and map these
        // window pointers to their specific input mappers on create.
        //
        static void mouse_button_callback(GLFWwindow*, int, int, int);
        static std::unordered_map<GLFWwindow*, InputMap*> callback_map;

        static void scroll_callback(GLFWwindow*, double, double);
        static void key_callback(GLFWwindow*, int, int, int, int);
        static void char_callback(GLFWwindow*, unsigned int);

        std::unordered_map<Input::Key, Input::State> key_state_map;
        std::unordered_map<Input::MouseButton, Input::State> mouse_button_state;
        std::unordered_multimap<std::string, Input::MouseButton> mouse_button_map;
        std::unordered_multimap<std::string, Input::Key> key_map;

        glm::vec2 scroll_offsets { 0.0f };

        bool mouse_locked { false };
        GLFWwindow* handle;
    };
}

#endif
