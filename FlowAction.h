#pragma once
#include <string>
#include <vector>

struct FlowAction {
    FlowAction(const std::string &name, const std::vector<std::string> &arguments, size_t layer) : name(name),
                                                                                                   arguments(arguments),
                                                                                                   layer(layer) {}

    std::string name;
    std::vector<std::string> arguments;
    size_t layer;

};

