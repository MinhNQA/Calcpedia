#include "handbook.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace {

std::string lower(std::string s) {
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

bool contains_ci(const std::string& hay, const std::string& needle) {
    return lower(hay).find(lower(needle)) != std::string::npos;
}

} // namespace

Handbook::Handbook() {
    auto add = [&](HandbookEntry e) { entries_.push_back(std::move(e)); };

    add({"+", {"add", "addition", "plus"}, "operator",
         "Adds two numbers.", "a + b", "3 + 4", {"-", "*", "sum"}});
    add({"-", {"subtract", "subtraction", "minus"}, "operator",
         "Subtracts the right operand from the left. Prefix minus negates a value.", "a - b   or   -a",
         "10 - 3", {"+", "*", "abs"}});
    add({"*", {"multiply", "multiplication", "times"}, "operator",
         "Multiplies two numbers. Binds tighter than + and -.", "a * b", "6 * 7", {"/", "^"}});
    add({"/", {"divide", "division"}, "operator",
         "Divides the left operand by the right. Division by zero is an error.", "a / b", "22 / 7",
         {"*", "%"}});
    add({"%", {"mod", "modulo", "remainder"}, "operator",
         "Remainder after division (floating-point modulo).", "a % b", "17 % 5", {"/"}});
    add({"^", {"pow", "power", "exponent"}, "operator",
         "Exponentiation, right-associative. Unary minus on the exponent is allowed: 2^-3.", "a ^ b",
         "2 ^ 10", {"pow", "exp", "sqrt"}});
    add({"!", {"factorial", "fact"}, "operator",
         "Postfix factorial. For a non-negative integer n, n! = 1·2·…·n. For real x it uses Gamma(x+1). "
         "Binds tighter than every binary operator.", "n!", "5!", {"gamma", "nCr", "nPr", "fact"}});
    add({"=", {"assign", "variable"}, "operator",
         "Stores a value in a named variable. Names are identifiers such as a, theta, x1.", "name = expr",
         "a = 5", {"ans"}});

    add({"pi", {"π"}, "constant",
         "The ratio of a circle's circumference to its diameter.", "pi ≈ 3.14159265359", "cos(pi)",
         {"e", "phi", "tau", "sin"}});
    add({"e", {"euler"}, "constant",
         "Euler's number, the base of the natural logarithm.", "e ≈ 2.71828182846", "ln(e)",
         {"pi", "exp", "ln"}});
    add({"phi", {"golden", "goldenratio"}, "constant",
         "The golden ratio (1+sqrt(5))/2, appearing in pentagons and Fibonacci limits.",
         "phi = (1 + sqrt(5)) / 2", "phi", {"sqrt", "pi"}});
    add({"tau", {}, "constant",
         "A full turn in radians: tau = 2*pi.", "tau = 2 * pi", "sin(tau / 4)", {"pi"}});
    add({"ans", {}, "constant",
         "The result of the previous successful calculation. Updated after each expression.", "ans",
         "ans + 1", {"="}});

    add({"sin", {"sine"}, "function",
         "Sine of an angle. Uses the current angle mode (radians or degrees).", "sin(x)", "sin(pi / 2)",
         {"cos", "tan", "asin"}});
    add({"cos", {"cosine"}, "function",
         "Cosine of an angle. Uses the current angle mode.", "cos(x)", "cos(0)", {"sin", "tan", "acos"}});
    add({"tan", {"tangent"}, "function",
         "Tangent of an angle: sin(x)/cos(x). Uses the current angle mode.", "tan(x)", "tan(pi / 4)",
         {"sin", "cos", "atan"}});
    add({"asin", {"arcsin", "arcsine"}, "function",
         "Inverse sine. Domain [-1, 1]. Result is in the current angle mode.", "asin(x)", "asin(1)",
         {"sin", "acos", "atan"}});
    add({"acos", {"arccos"}, "function",
         "Inverse cosine. Domain [-1, 1]. Result is in the current angle mode.", "acos(x)", "acos(-1)",
         {"cos", "asin"}});
    add({"atan", {"arctan"}, "function",
         "Inverse tangent. Result is in the current angle mode.", "atan(x)", "atan(1)", {"tan", "atan2"}});
    add({"atan2", {}, "function",
         "Two-argument inverse tangent of y/x, using signs of both to choose the quadrant.",
         "atan2(y, x)", "atan2(1, 1)", {"atan", "tan"}});
    add({"sinh", {}, "function", "Hyperbolic sine.", "sinh(x) = (e^x - e^(-x)) / 2", "sinh(0)",
         {"cosh", "tanh", "exp"}});
    add({"cosh", {}, "function", "Hyperbolic cosine.", "cosh(x) = (e^x + e^(-x)) / 2", "cosh(0)",
         {"sinh", "tanh"}});
    add({"tanh", {}, "function", "Hyperbolic tangent: sinh(x)/cosh(x).", "tanh(x)", "tanh(1)",
         {"sinh", "cosh"}});
    add({"exp", {"exponential"}, "function", "Natural exponential e^x.", "exp(x)", "exp(1)", {"e", "ln"}});
    add({"ln", {"logarithm", "naturallog"}, "function",
         "Natural logarithm (base e). Defined for x > 0.", "ln(x)", "ln(e)", {"log", "exp", "e"}});
    add({"log", {}, "function",
         "Common logarithm log10(x), or log(x, base) for an arbitrary base.", "log(x)   or   log(x, base)",
         "log(1000)", {"ln", "log10", "log2"}});
    add({"log10", {}, "function", "Base-10 logarithm.", "log10(x)", "log10(1000)", {"log", "ln"}});
    add({"log2", {}, "function", "Base-2 logarithm.", "log2(x)", "log2(8)", {"log", "ln"}});
    add({"sqrt", {"root", "squareroot"}, "function",
         "Principal square root. Defined for x >= 0.", "sqrt(x) = x^(1/2)", "sqrt(16)", {"cbrt", "^"}});
    add({"cbrt", {"cuberoot"}, "function", "Real cube root, defined for all real x.", "cbrt(x)", "cbrt(-8)",
         {"sqrt"}});
    add({"pow", {}, "function", "Function form of exponentiation.", "pow(a, b) = a ^ b", "pow(2, 10)",
         {"^", "exp"}});
    add({"hypot", {}, "function", "Euclidean norm sqrt(a^2 + b^2) without overflow.", "hypot(a, b)",
         "hypot(3, 4)", {"sqrt"}});
    add({"abs", {"absolute"}, "function", "Absolute value |x|.", "abs(x)", "abs(-7)", {"sign"}});
    add({"floor", {}, "function", "Greatest integer less than or equal to x.", "floor(x)", "floor(-1.2)",
         {"ceil", "round"}});
    add({"ceil", {}, "function", "Least integer greater than or equal to x.", "ceil(x)", "ceil(2.1)",
         {"floor", "round"}});
    add({"round", {}, "function", "Nearest integer, rounding halves away from zero in the usual C++ sense.",
         "round(x)", "round(2.5)", {"floor", "ceil"}});
    add({"sign", {"sgn"}, "function", "Returns -1, 0, or 1 according to the sign of x.", "sign(x)",
         "sign(-3.2)", {"abs"}});
    add({"min", {}, "function", "Smallest of two or more arguments.", "min(a, b, ...)", "min(3, -1, 4)",
         {"max"}});
    add({"max", {}, "function", "Largest of two or more arguments.", "max(a, b, ...)", "max(3, -1, 4)",
         {"min"}});
    add({"gamma", {"Γ"}, "function",
         "Euler's Gamma function. For positive integers, Gamma(n) = (n-1)!. Has poles at 0, -1, -2, ….",
         "Gamma(z) = ∫_0^∞ t^(z-1) e^(-t) dt", "gamma(5)", {"!", "fact", "factorial"}});
    add({"erf", {"errorfunction"}, "function",
         "Gauss error function, the integral of a Gaussian. erf(∞) = 1.",
         "erf(x) = (2/sqrt(pi)) ∫_0^x e^(-t^2) dt", "erf(0)", {"exp", "pi"}});
    add({"nCr", {"binomial", "choose", "combination", "combinations"}, "function",
         "Binomial coefficient: ways to choose k items from n. n and k must be integers.",
         "nCr(n, k) = n! / (k! (n-k)!)", "nCr(10, 3)", {"nPr", "!", "binomial"}});
    add({"nPr", {"permutation", "permutations"}, "function",
         "Permutations: ways to arrange k items from n. n and k must be integers.",
         "nPr(n, k) = n! / (n-k)!", "nPr(10, 3)", {"nCr", "!"}});
    add({"gcd", {"hcf", "greatestcommondivisor"}, "function",
         "Greatest common divisor of two integers.", "gcd(a, b)", "gcd(24, 18)", {"lcm"}});
    add({"lcm", {"leastcommonmultiple"}, "function",
         "Least common multiple of two integers.", "lcm(a, b) = |a b| / gcd(a, b)", "lcm(4, 6)", {"gcd"}});
    add({"fact", {}, "function", "Function form of factorial; same as the postfix ! operator.", "fact(n)",
         "fact(6)", {"!", "gamma"}});
    add({"deg", {"degree", "degrees"}, "function",
         "Converts radians to degrees. Also a command that switches the calculator into degree mode.",
         "deg(x) = x * 180 / pi", "deg(pi)", {"rad", "sin"}});
    add({"rad", {"radian", "radians"}, "function",
         "Converts degrees to radians. Also a command that switches the calculator into radian mode.",
         "rad(x) = x * pi / 180", "rad(180)", {"deg", "sin"}});

    add({"derivative", {"differentiate", "differentiation", "d/dx"}, "topic",
         "The derivative f'(x) is the instantaneous rate of change of f, the slope of the tangent line. "
         "Calcpedia does not differentiate symbolically; use small h to estimate: (f(x+h)-f(x))/h.",
         "(f(x + h) - f(x)) / h", "(sin(0.001) - sin(0)) / 0.001", {"limit", "integral", "sin"}});
    add({"integral", {"integrate", "antiderivative"}, "topic",
         "The definite integral is signed area under a curve. A crude Riemann sum is sum f(x)*dx.",
         "∫_a^b f(x) dx ≈ Σ f(x_i) Δx", "0.5 * (sin(0) + sin(1))", {"derivative", "limit"}});
    add({"limit", {}, "topic",
         "A limit describes the value a function approaches. Numerically, evaluate near the point from both sides.",
         "lim_{x→a} f(x)", "sin(0.0001) / 0.0001", {"derivative", "sin"}});
    add({"trigonometry", {"trig", "identity"}, "topic",
         "Trigonometric functions relate angles to ratios on the unit circle. Identities include "
         "sin^2 x + cos^2 x = 1.", "sin(x)^2 + cos(x)^2 = 1", "sin(pi/6)^2 + cos(pi/6)^2",
         {"sin", "cos", "tan"}});
    add({"logarithm", {"logs"}, "topic",
         "Logarithms invert exponentials. Change of base: log_b a = ln(a)/ln(b).",
         "log_b a = ln(a) / ln(b)", "ln(8) / ln(2)", {"ln", "log", "exp"}});
    add({"combinatorics", {"counting"}, "topic",
         "Counting collections and arrangements. Combinations nCr ignore order; permutations nPr keep it.",
         "nCr(n,k)  and  nPr(n,k)", "nCr(5, 2)", {"nCr", "nPr", "!"}});
    add({"plot", {"graph"}, "command",
         "Draw an ASCII graph of one or more expressions in x. Overlay several functions with commas. "
         "Optional range xmin xmax follows the expression(s).",
         "plot <expr>[, expr...] [xmin xmax]", "plot sin(x)/x -10 10", {"table", "sin"}});
    add({"table", {}, "command",
         "Print a table of values of an expression in x.", "table <expr> [xmin xmax [step]]",
         "table sin(x) 0 3 0.5", {"plot"}});
    add({"explain", {}, "command",
         "Annotate every symbol in an expression with its handbook definition, then show evaluation order.",
         "explain <expr>", "explain 5! + sqrt(16)/2", {"help", "try"}});
    add({"help", {}, "command",
         "Look up a handbook entry by name, operator, or topic.", "help <topic>", "help gamma",
         {"search", "list", "try"}});
    add({"search", {}, "command",
         "Find handbook entries whose names or text mention a word.", "search <query>", "search derivative",
         {"help", "list"}});
    add({"try", {}, "command",
         "Evaluate the example from a handbook entry in the live calculator.", "try <topic>", "try gamma",
         {"help", "explain"}});
}

const HandbookEntry* Handbook::find(const std::string& query) const {
    const std::string q = lower(query);
    for (const auto& e : entries_) {
        if (lower(e.id) == q) return &e;
        for (const auto& a : e.aliases) {
            if (lower(a) == q) return &e;
        }
    }
    return nullptr;
}

std::vector<const HandbookEntry*> Handbook::search(const std::string& query) const {
    std::vector<const HandbookEntry*> hits;
    if (query.empty()) return hits;
    for (const auto& e : entries_) {
        bool ok = contains_ci(e.id, query) || contains_ci(e.kind, query) ||
                  contains_ci(e.definition, query) || contains_ci(e.formula, query);
        for (const auto& a : e.aliases) ok = ok || contains_ci(a, query);
        for (const auto& r : e.related) ok = ok || contains_ci(r, query);
        if (ok) hits.push_back(&e);
    }
    return hits;
}

std::string Handbook::format(const HandbookEntry& e) const {
    std::ostringstream os;
    os << e.id << "  [" << e.kind << "]\n";
    if (!e.aliases.empty()) {
        os << "aliases: ";
        for (std::size_t i = 0; i < e.aliases.size(); ++i) {
            if (i) os << ", ";
            os << e.aliases[i];
        }
        os << "\n";
    }
    os << "\n" << e.definition << "\n";
    os << "\nformula:  " << e.formula << "\n";
    os << "example:  " << e.example << "\n";
    if (!e.related.empty()) {
        os << "related:  ";
        for (std::size_t i = 0; i < e.related.size(); ++i) {
            if (i) os << ", ";
            os << e.related[i];
        }
        os << "\n";
    }
    os << "\ntry " << e.id << "   runs the example in the calculator\n";
    return os.str();
}

std::string Handbook::format_help_index() const {
    return
        "Calcpedia — terminal calculator with a math handbook\n"
        "\n"
        "Calculate\n"
        "  2 + 3*4            operator precedence\n"
        "  (1+2)^3            parentheses\n"
        "  a = 5              variables; ans is the last result\n"
        "  pi  e  phi         constants\n"
        "  5!   nCr(10,3)     factorial, combinations, permutations\n"
        "  deg    rad         switch angle mode (also: mode)\n"
        "\n"
        "Visual\n"
        "  plot sin(x)/x -10 10\n"
        "  plot sin(x),cos(x) -6.3 6.3\n"
        "  table sin(x) 0 6 0.5\n"
        "\n"
        "Handbook\n"
        "  help [topic]       definition, formula, example, related\n"
        "  search <word>      look up by keyword\n"
        "  list               catalog of entries\n"
        "  try <topic>        run that entry's example\n"
        "\n"
        "Teach\n"
        "  explain <expr>     annotate symbols and show evaluation order\n"
        "\n"
        "Other: vars, clear, functions, quit\n";
}

std::string Handbook::format_list() const {
    std::ostringstream os;
    os << "Handbook entries:\n";
    for (const auto& e : entries_) {
        os << "  " << e.id;
        os << std::string(e.id.size() < 14 ? 14 - e.id.size() : 1, ' ');
        os << e.kind << " — " << e.definition.substr(0, 60);
        if (e.definition.size() > 60) os << "…";
        os << "\n";
    }
    return os.str();
}
