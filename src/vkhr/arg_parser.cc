#include <vkhr/arg_parser.hh>

namespace vkhr {
    ArgParser::ArgParser(int argc, char* argv[]) {
        parse(argc, argv);
    }

    void ArgParser::parse(int, char**) {
    }
}
