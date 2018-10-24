#include <vkhr/arg_parser.hh>

namespace vkhr {
    ArgParser::ArgParser(int argc, char** argv)
                        : argument_count { argc },
                          arguments { argv } { }

    const char* ArgParser::parse() {
        return nullptr;
    }

    int ArgParser::parse_integer() {
        return 0;
    }

    const char* ArgParser::parse_string() {
        return nullptr;
    }

    float ArgParser::parse_float() {
        return 0.0;
    }
}
