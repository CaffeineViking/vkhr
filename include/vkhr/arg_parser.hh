#ifndef VKHR_ARGUMENT_PARSER_HH
#define VKHR_ARGUMENT_PARSER_HH

#include <unordered_map>

#include <vector>
#include <string>

namespace vkhr {
    struct Argument {
        std::string name;

        enum class Type {
            Integer,
            Boolean,
            String,
            Floating
        } type;

        union Value {
            int integer;
            bool boolean;
            const char* string;
            float floating;
        } value;

        static Value make_integer(int integer);
        static Value make_boolean(bool boolean);
        static Value make_string(const char* string);
        static Value make_floating(float floating);

        std::string help;
    };

    class ArgParser final {
    public:
        ArgParser() = default;
        ArgParser(const std::vector<Argument>& arguments);

        std::string parse(int argc, char** argv);

        std::string get_help() const;

        const Argument& operator[](const std::string& name) const;

        bool add_argument(const Argument& argument);
        bool remove_argument(const std::string& name);

    private:
        const char* get_next_token();

        bool parse_integer(Argument& argument);
        bool parse_boolean(Argument& argument);
        bool parse_string(Argument& argument);
        bool parse_floating(Argument& argument);

        int current_argument { 1 };
        int argument_count { 0 };
        char** argument_values { nullptr };

        std::unordered_map<std::string,
                           Argument> arguments;
    };

    // List of arguments and default value.
    extern std::vector<Argument> arguments;
}

#endif
