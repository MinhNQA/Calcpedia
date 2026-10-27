#pragma once

#include "ast.hpp"
#include "eval.hpp"

#include <string>
#include <vector>

struct PlotSpec {
    std::vector<Expr> exprs;
    std::vector<std::string> labels;
    double xmin = -10;
    double xmax = 10;
    int width = 61;
    int height = 21;
};

std::string plot_ascii(Calculator& calc, const PlotSpec& spec);

std::string table_values(Calculator& calc, const Expr& expr, const std::string& label, double xmin,
                         double xmax, double step);
