#ifndef VKHR_ARGUMENT_PARSER_HH
#define VKHR_ARGUMENT_PARSER_HH

namespace vkhr {
    class ArgParser final {
    public:
        ArgParser() = default;
        ArgParser(int argc, char** argv);

        const char* parse();

        int parse_integer();
        const char* parse_string();
        float parse_float();

    private:
        int parsing { 1 };
        int argument_count { 0 };
        char** arguments { nullptr };
    };
}

#endif
