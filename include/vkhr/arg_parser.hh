#ifndef VKHR_ARGUMENT_PARSER_HH
#define VKHR_ARGUMENT_PARSER_HH

#include <string>
#include <unordered_map>

namespace vkhr {
    class ArgParser final {
    public:
        enum class Type {
            Integer,
            Boolean,
            String,
            Floating
        };

        union Value {
            int integer;
            bool boolean;
            const char* string;
            float floating;
        };

        using Map = std::unordered_map<std::string, Type>;

        ArgParser(const Map& argm);

        std::string parse(int argc, char** argv);

        Value& operator[](const std::string& name);

        const char* get_executed_command() const;

    private:
        const char* parse();

        int parse_int();
        bool parse_bool(const std::string& name);
        const char* parse_string();
        float parse_float();

        int current { 1 };
        int argument_count { 0 };
        char** arguments { nullptr };

        Map parameter_type_map;
        std::unordered_map<std::string, Value> value_map;
    };
}

#endif
