#pragma once

#include "ast.hpp"

#include <map>
#include <string>
#include <vector>

enum class AngleMode { Radians, Degrees };

struct EvalStep {
    std::string expr;
    double value;
};

struct EvalResult {
    double value = 0;
    std::vector<EvalStep> steps;
};

class Calculator {
public:
    Calculator();

    EvalResult evaluate(const Expr& e);
    EvalResult evaluate(const std::string& src);

    double evaluate_with_x(const Expr& e, double x);

    void set_angle_mode(AngleMode mode) { mode_ = mode; }
    AngleMode angle_mode() const { return mode_; }

    void set_var(const std::string& name, double value) { vars_[name] = value; }
    bool get_var(const std::string& name, double& out) const;
    const std::map<std::string, double>& vars() const { return vars_; }
    void clear_vars();

    double ans() const { return ans_; }

    static bool is_function(const std::string& name);
    static const std::vector<std::string>& function_names();

private:
    double eval_node(const Expr& e, std::vector<EvalStep>* steps);
    double call_function(const std::string& name, const std::vector<double>& args);
    double to_rad(double x) const;
    double from_rad(double x) const;

    std::map<std::string, double> vars_;
    std::map<std::string, double> consts_;
    AngleMode mode_ = AngleMode::Radians;
    double ans_ = 0;
};
