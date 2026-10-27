#pragma once

#include "eval.hpp"
#include "handbook.hpp"

#include <string>

std::string explain_expression(Calculator& calc, const Handbook& book, const std::string& src);
