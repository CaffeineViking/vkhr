#ifndef VKHR_ARGUMENT_HH
#define VKHR_ARGUMENT_HH

#include <string>
#include <vector>

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

    // List of arguments and default value.
    extern std::vector<Argument> arguments;
}

#endif
