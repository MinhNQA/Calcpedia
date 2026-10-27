#pragma once

#include <string>
#include <vector>

struct Expr {
    enum class Kind {
        Number,
        Variable,
        Unary,
        Binary,
        Factorial,
        Call,
        Assign
    };

    Kind kind = Kind::Number;
    std::string name;
    double number = 0;
    std::vector<Expr> kids;
    std::size_t start = 0;
    std::size_t end = 0;
    std::string source;
};

inline Expr make_number(double n, std::size_t start, std::size_t end, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Number;
    e.number = n;
    e.start = start;
    e.end = end;
    e.source = std::move(src);
    return e;
}

inline Expr make_var(std::string name, std::size_t start, std::size_t end, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Variable;
    e.name = std::move(name);
    e.start = start;
    e.end = end;
    e.source = std::move(src);
    return e;
}

inline Expr make_unary(std::string op, Expr inner, std::size_t start, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Unary;
    e.name = std::move(op);
    e.start = start;
    e.end = inner.end;
    e.source = std::move(src);
    e.kids.push_back(std::move(inner));
    return e;
}

inline Expr make_binary(std::string op, Expr a, Expr b, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Binary;
    e.name = std::move(op);
    e.start = a.start;
    e.end = b.end;
    e.source = std::move(src);
    e.kids.push_back(std::move(a));
    e.kids.push_back(std::move(b));
    return e;
}

inline Expr make_fact(Expr inner, std::size_t end, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Factorial;
    e.name = "!";
    e.start = inner.start;
    e.end = end;
    e.source = std::move(src);
    e.kids.push_back(std::move(inner));
    return e;
}

inline Expr make_call(std::string name, std::vector<Expr> args, std::size_t start, std::size_t end,
                      std::string src) {
    Expr e;
    e.kind = Expr::Kind::Call;
    e.name = std::move(name);
    e.start = start;
    e.end = end;
    e.source = std::move(src);
    e.kids = std::move(args);
    return e;
}

inline Expr make_assign(std::string name, Expr value, std::size_t start, std::string src) {
    Expr e;
    e.kind = Expr::Kind::Assign;
    e.name = std::move(name);
    e.start = start;
    e.end = value.end;
    e.source = std::move(src);
    e.kids.push_back(std::move(value));
    return e;
}
