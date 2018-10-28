#include <vkhr/argument.hh>

namespace vkhr {
    Argument::Value Argument::make_integer(int integer) {
        Value value;
        value.integer = integer;
        return value;
    }

    Argument::Value Argument::make_boolean(bool boolean) {
        Value value;
        value.boolean = boolean;
        return value;
    }

    Argument::Value Argument::make_string(const char* string) {
        Value value;
        value.string = string;
        return value;
    }

    Argument::Value Argument::make_floating(float floating) {
        Value value;
        value.floating = floating;
        return value;
    }

    std::vector<Argument> arguments {
        { "width",      Argument::Type::Integer, Argument::make_integer(1280),  "" },
        { "height",     Argument::Type::Integer, Argument::make_integer(720),   "" },
        { "cores",      Argument::Type::Integer, Argument::make_integer(8),     "" },

        { "fullscreen", Argument::Type::Boolean, Argument::make_boolean(false), "" },
        { "vsync",      Argument::Type::Boolean, Argument::make_boolean(true),  "" },
        { "profile",    Argument::Type::Boolean, Argument::make_boolean(false), "" },
        { "gui",        Argument::Type::Boolean, Argument::make_boolean(true),  "" },
        { "anti-alias", Argument::Type::Boolean, Argument::make_boolean(true),  "" },
        { "logging",    Argument::Type::Boolean, Argument::make_boolean(false), "" },

        { "device",     Argument::Type::String,  Argument::make_string("auto"), "" },
        { "assets",     Argument::Type::String,  Argument::make_string("auto"), "" }
    };
}
