#include <vkhr/arg_parser.hh>

#include <cstdlib>

namespace vkhr {
    ArgParser::ArgParser(const Map& argm)
                        : parameter_type_map { argm } {
        for (const auto& pair : argm) {
            Value value;
            switch (pair.second) {
            case Type::Integer:
                value.integer = 0;
                break;
            case Type::Boolean:
                value.boolean = 0;
                break;
            case Type::String:
                value.string = "";
                break;
            case Type::Floating:
                value.floating = 0.0;
                break;
            default: break;
            }

            value_map[pair.first] = value;
        }
    }

    std::string ArgParser::parse(int argc, char** argv) {
        this->current = 1;
        this->argument_count = argc;
        this->arguments = argv;

        std::string leftovers = "";

        while (auto parameter_name = parse()) {
            std::string name = parameter_name;

            if (name.substr(0, 2) != "--") {
                leftovers.append(name);
                continue;
            }

            name = name.substr(2, std::string::npos);
            auto parameter = parameter_type_map.find(name);
            auto boolean_p = parameter_type_map.end();
            std::string boolean_name = "";

            if (name.length() > 3) {
                boolean_name = name.substr(3, std::string::npos);
                boolean_p = parameter_type_map.find(boolean_name);
            }

            if (parameter != parameter_type_map.end() ||
                boolean_p != parameter_type_map.end()) {

                Type type;

                if (boolean_p != parameter_type_map.end()) {
                    type = boolean_p->second;
                } else {
                    type = parameter->second;
                }

                Value parameter_value;

                switch (type) {
                case Type::Integer:
                    parameter_value.integer  = parse_int();
                    break;
                case Type::Boolean:
                    parameter_value.boolean  = parse_bool(name);
                    break;
                case Type::String:
                    parameter_value.string   = parse_string();
                    break;
                case Type::Floating:
                    parameter_value.floating = parse_float();
                    break;
                default: break;
                }

                if (type == Type::Boolean) {
                    if (parameter_value.boolean == false) {
                        value_map[boolean_name] = parameter_value;
                    } else {
                        value_map[name] = parameter_value;
                    }
                } else {
                    value_map[name] = parameter_value;
                }
            }
        }

        return leftovers;
    }

    const char* ArgParser::get_executed_command() const {
        return arguments[0];
    }

    const char* ArgParser::parse() {
        if (current >= argument_count)
            return nullptr;
        return arguments[current++];
    }

    ArgParser::Value& ArgParser::operator[](const std::string& name) {
        return value_map.at(name);
    }

    int ArgParser::parse_int() {
        return std::atoi(arguments[current++]);
    }

    bool ArgParser::parse_bool(const std::string& name) {
        if (name.length() > 3) {
            auto value = name.substr(0, 2);
            if (value == "no") return false;
            else return true;
        } else return true;
    }

    const char* ArgParser::parse_string() {
        return arguments[current++];
    }

    float ArgParser::parse_float() {
        return std::atof(arguments[current++]);
    }
}
