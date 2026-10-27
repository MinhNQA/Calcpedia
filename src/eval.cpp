#include "eval.hpp"

#include "error.hpp"
#include "fmt.hpp"
#include "parser.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <numeric>
#include <utility>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kE = 2.71828182845904523536;
constexpr double kPhi = 1.61803398874989484820;

bool near_int(double x) {
    return std::isfinite(x) && std::fabs(x - std::round(x)) < 1e-9;
}

long long to_int(double x, const char* what) {
    if (!near_int(x)) throw CalcError(std::string(what) + " requires an integer");
    return static_cast<long long>(std::llround(x));
}

double factorial_n(double n) {
    if (n < 0 && near_int(n)) throw CalcError("factorial is undefined for negative integers");
    if (near_int(n) && n >= 0) {
        const long long k = static_cast<long long>(std::llround(n));
        if (k > 170) throw CalcError("factorial overflow");
        double acc = 1;
        for (long long i = 2; i <= k; ++i) acc *= static_cast<double>(i);
        return acc;
    }
    const double g = std::tgamma(n + 1.0);
    if (!std::isfinite(g)) throw CalcError("factorial overflow or undefined");
    return g;
}

double nCr(double n, double k) {
    const long long N = to_int(n, "nCr");
    const long long K = to_int(k, "nCr");
    if (N < 0 || K < 0 || K > N) return 0;
    long long kk = std::min(K, N - K);
    double acc = 1;
    for (long long i = 1; i <= kk; ++i) {
        acc *= static_cast<double>(N - kk + i);
        acc /= static_cast<double>(i);
    }
    return std::round(acc);
}

double nPr(double n, double k) {
    const long long N = to_int(n, "nPr");
    const long long K = to_int(k, "nPr");
    if (N < 0 || K < 0 || K > N) return 0;
    double acc = 1;
    for (long long i = 0; i < K; ++i) acc *= static_cast<double>(N - i);
    return acc;
}

double gcd_d(double a, double b) {
    long long A = to_int(a, "gcd");
    long long B = to_int(b, "gcd");
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    return static_cast<double>(std::gcd(A, B));
}

double lcm_d(double a, double b) {
    long long A = to_int(a, "lcm");
    long long B = to_int(b, "lcm");
    if (A < 0) A = -A;
    if (B < 0) B = -B;
    if (A == 0 || B == 0) return 0;
    const long long g = std::gcd(A, B);
    return static_cast<double>(A / g * B);
}

void expect_arity(const std::string& name, const std::vector<double>& args, std::size_t n) {
    if (args.size() != n) {
        throw CalcError(name + " expects " + std::to_string(n) + " argument" + (n == 1 ? "" : "s"));
    }
}

void expect_arity_range(const std::string& name, const std::vector<double>& args, std::size_t lo,
                        std::size_t hi) {
    if (args.size() < lo || args.size() > hi) {
        throw CalcError(name + " has wrong number of arguments");
    }
}

const std::vector<std::string> kFunctions = {
    "sin",  "cos",  "tan",   "asin", "acos", "atan", "atan2", "sinh", "cosh", "tanh",
    "exp",  "ln",   "log",   "log10","log2", "sqrt", "cbrt",  "pow",  "hypot","abs",
    "floor","ceil", "round", "sign", "min",  "max",  "gamma", "erf",  "nCr",  "nPr",
    "gcd",  "lcm",  "fact",  "deg",  "rad"};

} // namespace

Calculator::Calculator() {
    consts_["pi"] = kPi;
    consts_["e"] = kE;
    consts_["phi"] = kPhi;
    consts_["tau"] = 2.0 * kPi;
}

bool Calculator::is_function(const std::string& name) {
    return std::find(kFunctions.begin(), kFunctions.end(), name) != kFunctions.end();
}

const std::vector<std::string>& Calculator::function_names() { return kFunctions; }

bool Calculator::get_var(const std::string& name, double& out) const {
    auto it = vars_.find(name);
    if (it == vars_.end()) return false;
    out = it->second;
    return true;
}

void Calculator::clear_vars() { vars_.clear(); }

double Calculator::to_rad(double x) const {
    return mode_ == AngleMode::Degrees ? x * kPi / 180.0 : x;
}

double Calculator::from_rad(double x) const {
    return mode_ == AngleMode::Degrees ? x * 180.0 / kPi : x;
}

EvalResult Calculator::evaluate(const std::string& src) { return evaluate(parse_expr(src)); }

EvalResult Calculator::evaluate(const Expr& e) {
    EvalResult r;
    r.value = eval_node(e, &r.steps);
    ans_ = r.value;
    vars_["ans"] = ans_;
    return r;
}

double Calculator::evaluate_with_x(const Expr& e, double x) {
    const auto saved = vars_.find("x");
    const bool had = saved != vars_.end();
    const double old = had ? saved->second : 0;
    vars_["x"] = x;
    try {
        const double v = eval_node(e, nullptr);
        if (had) vars_["x"] = old;
        else vars_.erase("x");
        return v;
    } catch (...) {
        if (had) vars_["x"] = old;
        else vars_.erase("x");
        throw;
    }
}

double Calculator::eval_node(const Expr& e, std::vector<EvalStep>* steps) {
    auto record = [&](double v) {
        if (steps && e.kind != Expr::Kind::Number && e.kind != Expr::Kind::Variable) {
            std::string text = e.source;
            while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.erase(text.begin());
            while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.pop_back();
            if (text.empty()) text = "?";
            steps->push_back(EvalStep{text, v});
        }
        return v;
    };

    switch (e.kind) {
        case Expr::Kind::Number:
            return e.number;
        case Expr::Kind::Variable: {
            if (e.name == "ans") return ans_;
            auto cit = consts_.find(e.name);
            if (cit != consts_.end()) return cit->second;
            auto vit = vars_.find(e.name);
            if (vit != vars_.end()) return vit->second;
            throw CalcError("unknown identifier '" + e.name + "'");
        }
        case Expr::Kind::Unary: {
            const double x = eval_node(e.kids[0], steps);
            const double v = (e.name == "-") ? -x : x;
            return record(v);
        }
        case Expr::Kind::Binary: {
            const double a = eval_node(e.kids[0], steps);
            const double b = eval_node(e.kids[1], steps);
            double v = 0;
            if (e.name == "+") v = a + b;
            else if (e.name == "-") v = a - b;
            else if (e.name == "*") v = a * b;
            else if (e.name == "/") {
                if (b == 0) throw CalcError("division by zero");
                v = a / b;
            } else if (e.name == "%") {
                if (b == 0) throw CalcError("modulo by zero");
                v = std::fmod(a, b);
            } else if (e.name == "^") {
                v = std::pow(a, b);
                if (!std::isfinite(v)) throw CalcError("power overflow or undefined");
            } else {
                throw CalcError("unknown operator '" + e.name + "'");
            }
            return record(v);
        }
        case Expr::Kind::Factorial: {
            const double x = eval_node(e.kids[0], steps);
            return record(factorial_n(x));
        }
        case Expr::Kind::Call: {
            std::vector<double> args;
            args.reserve(e.kids.size());
            for (const auto& k : e.kids) args.push_back(eval_node(k, steps));
            return record(call_function(e.name, args));
        }
        case Expr::Kind::Assign: {
            if (consts_.count(e.name) || e.name == "ans") {
                throw CalcError("cannot assign to '" + e.name + "'");
            }
            const double v = eval_node(e.kids[0], steps);
            vars_[e.name] = v;
            return record(v);
        }
    }
    throw CalcError("internal evaluator error");
}

double Calculator::call_function(const std::string& raw_name, const std::vector<double>& args) {
    std::string name = raw_name;
    for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (name == "ncr") name = "nCr";
    if (name == "npr") name = "nPr";
    if (name == "c") name = "nCr";
    if (name == "p") name = "nPr";
    auto one = [&]() {
        expect_arity(name, args, 1);
        return args[0];
    };
    auto two = [&]() {
        expect_arity(name, args, 2);
        return std::pair<double, double>{args[0], args[1]};
    };

    if (name == "sin") return std::sin(to_rad(one()));
    if (name == "cos") return std::cos(to_rad(one()));
    if (name == "tan") return std::tan(to_rad(one()));
    if (name == "asin") {
        const double x = one();
        if (x < -1 || x > 1) throw CalcError("asin domain is [-1, 1]");
        return from_rad(std::asin(x));
    }
    if (name == "acos") {
        const double x = one();
        if (x < -1 || x > 1) throw CalcError("acos domain is [-1, 1]");
        return from_rad(std::acos(x));
    }
    if (name == "atan") return from_rad(std::atan(one()));
    if (name == "atan2") {
        const auto p = two();
        return from_rad(std::atan2(p.first, p.second));
    }
    if (name == "sinh") return std::sinh(one());
    if (name == "cosh") return std::cosh(one());
    if (name == "tanh") return std::tanh(one());
    if (name == "exp") return std::exp(one());
    if (name == "ln") {
        const double x = one();
        if (x <= 0) throw CalcError("ln is defined for x > 0");
        return std::log(x);
    }
    if (name == "log") {
        expect_arity_range(name, args, 1, 2);
        if (args.size() == 1) {
            if (args[0] <= 0) throw CalcError("log is defined for x > 0");
            return std::log10(args[0]);
        }
        if (args[0] <= 0 || args[1] <= 0 || args[1] == 1) {
            throw CalcError("log(x, base) needs x > 0 and base > 0, base != 1");
        }
        return std::log(args[0]) / std::log(args[1]);
    }
    if (name == "log10") {
        const double x = one();
        if (x <= 0) throw CalcError("log10 is defined for x > 0");
        return std::log10(x);
    }
    if (name == "log2") {
        const double x = one();
        if (x <= 0) throw CalcError("log2 is defined for x > 0");
        return std::log2(x);
    }
    if (name == "sqrt") {
        const double x = one();
        if (x < 0) throw CalcError("sqrt is defined for x >= 0");
        return std::sqrt(x);
    }
    if (name == "cbrt") return std::cbrt(one());
    if (name == "pow") {
        const auto p = two();
        const double v = std::pow(p.first, p.second);
        if (!std::isfinite(v)) throw CalcError("pow overflow or undefined");
        return v;
    }
    if (name == "hypot") {
        const auto p = two();
        return std::hypot(p.first, p.second);
    }
    if (name == "abs") return std::fabs(one());
    if (name == "floor") return std::floor(one());
    if (name == "ceil") return std::ceil(one());
    if (name == "round") return std::round(one());
    if (name == "sign") {
        const double x = one();
        if (x > 0) return 1;
        if (x < 0) return -1;
        return 0;
    }
    if (name == "min") {
        expect_arity_range(name, args, 2, 8);
        return *std::min_element(args.begin(), args.end());
    }
    if (name == "max") {
        expect_arity_range(name, args, 2, 8);
        return *std::max_element(args.begin(), args.end());
    }
    if (name == "gamma") {
        const double x = one();
        const double g = std::tgamma(x);
        if (!std::isfinite(g)) throw CalcError("gamma overflow or undefined (non-positive integer)");
        return g;
    }
    if (name == "erf") return std::erf(one());
    if (name == "nCr") {
        const auto p = two();
        return nCr(p.first, p.second);
    }
    if (name == "nPr") {
        const auto p = two();
        return nPr(p.first, p.second);
    }
    if (name == "gcd") {
        const auto p = two();
        return gcd_d(p.first, p.second);
    }
    if (name == "lcm") {
        const auto p = two();
        return lcm_d(p.first, p.second);
    }
    if (name == "fact") return factorial_n(one());
    if (name == "deg") {
        // Convert radians to degrees (independent of mode).
        return one() * 180.0 / kPi;
    }
    if (name == "rad") {
        return one() * kPi / 180.0;
    }
    throw CalcError("unknown function '" + name + "'");
}
