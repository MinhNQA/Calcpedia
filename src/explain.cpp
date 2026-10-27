#include "explain.hpp"

#include "fmt.hpp"
#include "parser.hpp"

#include <cctype>
#include <set>
#include <sstream>
#include <vector>

namespace {

struct Note {
    std::string symbol;
    std::string text;
};

void collect(const Expr& e, const Handbook& book, std::vector<Note>& notes, std::set<std::string>& seen) {
    auto add = [&](const std::string& symbol) {
        if (!seen.insert(symbol).second) return;
        Note n;
        n.symbol = symbol;
        if (const HandbookEntry* ent = book.find(symbol)) {
            n.text = ent->definition;
        } else if (symbol.size() && std::isdigit(static_cast<unsigned char>(symbol[0]))) {
            n.text = "numeric literal";
        } else {
            n.text = "identifier (variable, or unknown)";
        }
        notes.push_back(std::move(n));
    };

    switch (e.kind) {
        case Expr::Kind::Number:
            add(e.source.empty() ? format_number(e.number) : e.source);
            break;
        case Expr::Kind::Variable:
            add(e.name);
            break;
        case Expr::Kind::Unary:
            add(e.name);
            collect(e.kids[0], book, notes, seen);
            break;
        case Expr::Kind::Binary:
            collect(e.kids[0], book, notes, seen);
            add(e.name);
            collect(e.kids[1], book, notes, seen);
            break;
        case Expr::Kind::Factorial:
            collect(e.kids[0], book, notes, seen);
            add("!");
            break;
        case Expr::Kind::Call:
            add(e.name);
            for (const auto& k : e.kids) collect(k, book, notes, seen);
            break;
        case Expr::Kind::Assign:
            add(e.name);
            add("=");
            collect(e.kids[0], book, notes, seen);
            break;
    }
}

} // namespace

std::string explain_expression(Calculator& calc, const Handbook& book, const std::string& src) {
    const Expr tree = parse_expr(src);
    std::vector<Note> notes;
    std::set<std::string> seen;
    collect(tree, book, notes, seen);

    const EvalResult result = calc.evaluate(tree);

    std::ostringstream os;
    os << "expression:  " << src << "\n\n";
    os << "symbols\n";
    for (const auto& n : notes) {
        os << "  " << n.symbol;
        const std::size_t pad = n.symbol.size() < 10 ? 10 - n.symbol.size() : 1;
        os << std::string(pad, ' ') << n.text << "\n";
    }
    os << "\nevaluation order\n";
    if (result.steps.empty()) {
        os << "  " << src << "  =  " << format_number(result.value) << "  (literal or variable)\n";
    } else {
        int i = 1;
        for (const auto& s : result.steps) {
            os << "  " << i++ << ".  " << s.expr << "  =  " << format_number(s.value) << "\n";
        }
    }
    os << "\nresult:  " << format_number(result.value) << "\n";
    return os.str();
}
