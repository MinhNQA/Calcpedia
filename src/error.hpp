#pragma once

#include <stdexcept>
#include <string>

struct CalcError : std::runtime_error {
    explicit CalcError(const std::string& msg) : std::runtime_error(msg) {}
};
