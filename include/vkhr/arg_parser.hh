#ifndef VKHR_ARGUMENT_PARSER_HH
#define VKHR_ARGUMENT_PARSER_HH

namespace vkhr {
    class ArgParser final {
    public:
        ArgParser() = default;
        ArgParser(int argc, char* argv[]);

        void parse(int argc, char** argv);

    private:
    };
}

#endif
