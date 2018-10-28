#ifndef VKHR_ARGUMENT_PARSER_HH
#define VKHR_ARGUMENT_PARSER_HH

#include <vkhr/argument.hh>

#include <unordered_map>

#include <vector>
#include <string>

namespace vkhr {
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
}

#endif
