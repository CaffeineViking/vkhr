#ifndef VKHR_ARGUMENTS_HH
#define VKHR_ARGUMENTS_HH

#include <vkhr/arg_parser.hh>

namespace vkhr {
    static ArgParser::Map arguments {
        { "width", ArgParser::Type::Integer },
        { "height", ArgParser::Type::Integer },
        { "fullscreen", ArgParser::Type::Boolean },
        { "vsync", ArgParser::Type::Boolean },
        { "profile", ArgParser::Type::Boolean },
        { "gui", ArgParser::Type::Boolean },
    };

    static void populate_with_default_values(ArgParser& argp) {
        argp["width"].integer      = 1280;
        argp["height"].integer     = 720;
        argp["fullscreen"].boolean = false;
        argp["vsync"].boolean      = true;
        argp["profile"].boolean    = false;
        argp["gui"].boolean        = true;
    }
}

#endif
