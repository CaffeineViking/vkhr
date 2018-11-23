#include <vkhr/arg_parser.hh>

#include <cstdlib>

namespace vkhr {
    ArgParser::ArgParser(const std::vector<Argument>& arguments) {
        for (const auto& argument : arguments) {
            add_argument(argument);
        }
    }

    std::string ArgParser::parse(int argc, char** argv) {
        current_argument = 1;
        argument_count  = argc;
        argument_values = argv;

        std::string parse_tail;

        while (auto token = get_next_token()) {
            std::string token_string { token };

            std::string argument_name;

            if (token_string.length() > 2) {
                auto magic = token_string.substr(0, 2);

                if (magic != "--") {
                    parse_tail = token_string;
                    break;
                }

                argument_name = token_string.substr(2);
            } else {
                parse_tail = token_string;
                break;
            }

            auto argument = arguments.find(argument_name);
            if (argument != arguments.end()) {

                switch (argument->second.type) {
                case Argument::Type::Integer:
                    parse_integer(argument->second);
                    break;
                case Argument::Type::Boolean:
                    parse_boolean(argument->second);
                    break;
                case Argument::Type::String:
                    parse_string(argument->second);
                    break;
                case Argument::Type::Floating:
                    parse_floating(argument->second);
                    break;
                default: break;
                }

            }
        }

        return parse_tail;
    }

    std::string ArgParser::get_help() const {
        return "";
    }

    const Argument& ArgParser::operator[](const std::string& name) const {
        if (name == "x")      return arguments.at("width");
        else if (name == "y") return arguments.at("height");
        else                  return arguments.at(name);
    }

    bool ArgParser::add_argument(const Argument& argument) {
        return arguments.insert({ argument.name, argument }).second;
    }

    bool ArgParser::remove_argument(const std::string& name) {
        auto argument_position = arguments.find(name);
        if (argument_position != arguments.end()) {
            arguments.erase(argument_position);
            return true;
        }

        return false;
    }

    const char* ArgParser::get_next_token() {
        if (current_argument > argument_count) {
            return nullptr;
        } else {
            return argument_values[current_argument++];
        }
    }

    bool ArgParser::parse_integer(Argument& argument) {
        auto integer = get_next_token();

        if (integer != nullptr) {
            argument.value.integer = std::atoi(integer);
            return true;
        }

        return false;
    }

    bool ArgParser::parse_boolean(Argument& argument) {
        auto boolean = get_next_token();

        if (boolean != nullptr) {
            std::string boolean_string { boolean };

            if (boolean_string == "on" ||
                boolean_string == "yes") {
                argument.value.boolean = true;
            } else if (boolean_string == "off" ||
                       boolean_string == "no") {
                argument.value.boolean = false;
            } else return false;

            return true;
        }

        return false;
    }

    bool ArgParser::parse_string(Argument& argument) {
        auto string = get_next_token();

        if (string != nullptr) {
            argument.value.string = string;
            return true;
        }

        return false;
    }

    bool ArgParser::parse_floating(Argument& argument) {
        auto floating = get_next_token();

        if (floating != nullptr) {
            argument.value.floating = std::atof(floating);
            return true;
        }

        return false;
    }

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
        { "ui",         Argument::Type::Boolean, Argument::make_boolean(true),  "" },
        { "anti-alias", Argument::Type::Boolean, Argument::make_boolean(false), "" },
    };
}
