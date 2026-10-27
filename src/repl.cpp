#include "repl.hpp"

#include "error.hpp"
#include "explain.hpp"
#include "fmt.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "plot.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

std::string trim(std::string s) {
    std::size_t a = 0;
    while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    std::size_t b = s.size();
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string first_word(const std::string& s, std::string& rest) {
    std::size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    const std::size_t start = i;
    while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    rest = trim(s.substr(i));
    return s.substr(start, i - start);
}

// Trailing numeric arguments (optional sign) after an expression.
std::vector<double> peel_trailing_numbers(const std::vector<Token>& tokens, int max_count,
                                          int& expr_last) {
    std::vector<double> nums;
    int i = static_cast<int>(tokens.size()) - 2; // skip End
    expr_last = i;

    auto take = [&]() -> bool {
        if (i < 0 || tokens[static_cast<std::size_t>(i)].kind != TokenKind::Number) return false;
        double sign = 1;
        int nidx = i;
        int start = i;
        if (i >= 1) {
            const TokenKind s = tokens[static_cast<std::size_t>(i - 1)].kind;
            if (s == TokenKind::Minus || s == TokenKind::Plus) {
                if (s == TokenKind::Minus) sign = -1;
                start = i - 1;
            }
        }
        if (start <= 0 && nums.size() + 1 >= 2) {
            // Need at least one token left for the expression when peeling a range.
        }
        nums.push_back(sign * tokens[static_cast<std::size_t>(nidx)].number);
        i = start - 1;
        return true;
    };

    while (static_cast<int>(nums.size()) < max_count && take()) {
    }
    std::reverse(nums.begin(), nums.end());
    expr_last = i;
    return nums;
}

void split_expr_list(const std::string& expr_src, std::vector<std::string>& exprs) {
    exprs.clear();
    std::string cur;
    int depth = 0;
    for (char c : expr_src) {
        if (c == '(') ++depth;
        if (c == ')') --depth;
        if (c == ',' && depth == 0) {
            cur = trim(cur);
            if (cur.empty()) throw CalcError("empty expression in list");
            exprs.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    cur = trim(cur);
    if (cur.empty()) throw CalcError("expected an expression");
    exprs.push_back(cur);
}

void split_plot_args(const std::string& arg, std::vector<std::string>& exprs, std::vector<double>& range) {
    if (trim(arg).empty()) throw CalcError("expected an expression in x");
    const std::vector<Token> tokens = Lexer(arg).tokenize();
    int expr_last = 0;
    range = peel_trailing_numbers(tokens, 3, expr_last);
    if (range.size() == 1) {
        // A single trailing number is part of the expression (e.g. x-10).
        range.clear();
        expr_last = static_cast<int>(tokens.size()) - 2;
    }
    if (expr_last < 0) throw CalcError("expected an expression in x");
    std::string head = trim(arg.substr(0, tokens[static_cast<std::size_t>(expr_last)].end));
    split_expr_list(head, exprs);
}

} // namespace

std::string Repl::cmd_help(const std::string& arg) {
    if (arg.empty()) return book_.format_help_index();
    const HandbookEntry* e = book_.find(arg);
    if (!e) {
        const auto hits = book_.search(arg);
        std::ostringstream os;
        os << "no exact match for '" << arg << "'";
        if (!hits.empty()) {
            os << "; nearby: ";
            for (std::size_t i = 0; i < hits.size() && i < 8; ++i) {
                if (i) os << ", ";
                os << hits[i]->id;
            }
        }
        os << "\n";
        return os.str();
    }
    last_topic_ = e->id;
    return book_.format(*e);
}

std::string Repl::cmd_search(const std::string& arg) {
    if (arg.empty()) return "usage: search <query>\n";
    const auto hits = book_.search(arg);
    if (hits.empty()) return "no handbook entries for '" + arg + "'\n";
    std::ostringstream os;
    os << hits.size() << " hit" << (hits.size() == 1 ? "" : "s") << " for '" << arg << "':\n";
    for (const auto* e : hits) {
        os << "  " << e->id << "  [" << e->kind << "]\n";
    }
    return os.str();
}

std::string Repl::cmd_try(const std::string& arg) {
    std::string topic = arg.empty() ? last_topic_ : arg;
    if (topic.empty()) return "usage: try <topic>   (or open an entry with help first)\n";
    const HandbookEntry* e = book_.find(topic);
    if (!e) return "unknown topic '" + topic + "'\n";
    last_topic_ = e->id;
    std::ostringstream os;
    os << "try " << e->id << ":  " << e->example << "\n";
    try {
        const EvalResult r = calc_.evaluate(e->example);
        os << "  = " << format_number(r.value) << "\n";
    } catch (const std::exception& ex) {
        os << "  error: " << ex.what() << "\n";
    }
    return os.str();
}

std::string Repl::cmd_plot(const std::string& arg) {
    std::vector<std::string> labels;
    std::vector<double> range;
    split_plot_args(arg, labels, range);
    PlotSpec spec;
    spec.labels = labels;
    if (range.size() >= 2) {
        spec.xmin = range[0];
        spec.xmax = range[1];
    }
    for (const auto& s : labels) spec.exprs.push_back(parse_expr(s));
    return plot_ascii(calc_, spec);
}

std::string Repl::cmd_table(const std::string& arg) {
    std::vector<std::string> labels;
    std::vector<double> range;
    split_plot_args(arg, labels, range);
    if (labels.size() != 1) throw CalcError("table takes one expression in x");
    double xmin = 0, xmax = 10, step = 1;
    if (range.size() >= 2) {
        xmin = range[0];
        xmax = range[1];
        step = (xmax - xmin) / 10.0;
        if (!(step > 0)) step = 1;
    }
    if (range.size() >= 3) step = range[2];
    const Expr e = parse_expr(labels[0]);
    return table_values(calc_, e, labels[0], xmin, xmax, step);
}

std::string Repl::cmd_vars() const {
    std::ostringstream os;
    os << "mode: " << (calc_.angle_mode() == AngleMode::Degrees ? "degrees" : "radians") << "\n";
    os << "ans = " << format_number(calc_.ans()) << "\n";
    if (calc_.vars().empty()) {
        os << "(no user variables)\n";
        return os.str();
    }
    for (const auto& kv : calc_.vars()) {
        if (kv.first == "ans") continue;
        os << kv.first << " = " << format_number(kv.second) << "\n";
    }
    return os.str();
}

std::string Repl::handle_line(const std::string& raw) {
    const std::string line = trim(raw);
    if (line.empty() || line[0] == '#') return "";

    std::string rest;
    const std::string cmd = first_word(line, rest);
    const std::string cmd_l = [&] {
        std::string s = cmd;
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }();

    if (cmd_l == "quit" || cmd_l == "exit") return "__QUIT__";
    if (cmd_l == "help" || cmd_l == "?") return cmd_help(rest);
    if (cmd_l == "search") return cmd_search(rest);
    if (cmd_l == "list") return book_.format_list();
    if (cmd_l == "try") return cmd_try(rest);
    if (cmd_l == "explain") {
        if (rest.empty()) return "usage: explain <expr>\n";
        return explain_expression(calc_, book_, rest);
    }
    if (cmd_l == "plot") return cmd_plot(rest);
    if (cmd_l == "table") return cmd_table(rest);
    if (cmd_l == "vars") return cmd_vars();
    if (cmd_l == "clear") {
        calc_.clear_vars();
        return "variables cleared\n";
    }
    if (cmd_l == "deg" || cmd_l == "degrees") {
        calc_.set_angle_mode(AngleMode::Degrees);
        return "angle mode: degrees\n";
    }
    if (cmd_l == "rad" || cmd_l == "radians") {
        calc_.set_angle_mode(AngleMode::Radians);
        return "angle mode: radians\n";
    }
    if (cmd_l == "mode") {
        std::ostringstream os;
        os << "angle mode: " << (calc_.angle_mode() == AngleMode::Degrees ? "degrees" : "radians") << "\n";
        return os.str();
    }
    if (cmd_l == "functions") {
        std::ostringstream os;
        os << "functions:\n";
        int n = 0;
        for (const auto& f : Calculator::function_names()) {
            os << "  " << f;
            if (++n % 7 == 0) os << "\n";
        }
        if (n % 7) os << "\n";
        os << "constants: pi, e, phi, tau, ans\n";
        return os.str();
    }

    const EvalResult r = calc_.evaluate(line);
    return format_number(r.value) + "\n";
}

int Repl::run(std::istream& in, std::ostream& out, bool interactive) {
    if (interactive) {
        out << "Calcpedia  (type help, quit to exit)\n";
    }
    std::string line;
    while (true) {
        if (interactive) {
            out << "> ";
            out.flush();
        }
        if (!std::getline(in, line)) break;
        try {
            const std::string result = handle_line(line);
            if (result == "__QUIT__") break;
            out << result;
            if (!result.empty() && result.back() != '\n') out << "\n";
        } catch (const std::exception& ex) {
            out << "error: " << ex.what() << "\n";
        }
    }
    return 0;
}
