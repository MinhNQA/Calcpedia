#pragma once

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"

#include <utility>

class Parser {
public:
    explicit Parser(std::string src) : src_(std::move(src)), tokens_(Lexer(src_).tokenize()) {}

    Expr parse() {
        Expr e = parse_assign();
        if (!check(TokenKind::End)) {
            throw CalcError("unexpected token '" + peek().text + "'");
        }
        return e;
    }

    static std::vector<Expr> parse_comma_list(const std::string& src) {
        Parser p(src);
        std::vector<Expr> out;
        if (p.check(TokenKind::End)) return out;
        out.push_back(p.parse_assign());
        while (p.match(TokenKind::Comma)) {
            out.push_back(p.parse_assign());
        }
        if (!p.check(TokenKind::End)) {
            throw CalcError("unexpected token '" + p.peek().text + "'");
        }
        return out;
    }

private:
    std::string src_;
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;

    const Token& peek() const { return tokens_[pos_]; }

    bool check(TokenKind k) const { return peek().kind == k; }

    bool match(TokenKind k) {
        if (check(k)) {
            ++pos_;
            return true;
        }
        return false;
    }

    Token consume(TokenKind k, const char* what) {
        if (!check(k)) throw CalcError(what);
        return tokens_[pos_++];
    }

    std::string slice(std::size_t start, std::size_t end) const {
        if (end > src_.size()) end = src_.size();
        if (start > end) start = end;
        return src_.substr(start, end - start);
    }

    Expr parse_assign() {
        if (check(TokenKind::Ident) && pos_ + 1 < tokens_.size() &&
            tokens_[pos_ + 1].kind == TokenKind::Equal) {
            const Token id = tokens_[pos_++];
            ++pos_; // '='
            Expr rhs = parse_assign();
            const std::size_t end = rhs.end;
            return make_assign(id.text, std::move(rhs), id.start, slice(id.start, end));
        }
        return parse_add();
    }

    Expr parse_add() {
        Expr left = parse_mul();
        while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
            const Token op = tokens_[pos_++];
            Expr right = parse_mul();
            const std::size_t start = left.start;
            const std::size_t end = right.end;
            left = make_binary(op.text, std::move(left), std::move(right), slice(start, end));
        }
        return left;
    }

    Expr parse_mul() {
        Expr left = parse_unary();
        while (check(TokenKind::Star) || check(TokenKind::Slash) || check(TokenKind::Percent)) {
            const Token op = tokens_[pos_++];
            Expr right = parse_unary();
            const std::size_t start = left.start;
            const std::size_t end = right.end;
            left = make_binary(op.text, std::move(left), std::move(right), slice(start, end));
        }
        return left;
    }

    Expr parse_unary() {
        if (check(TokenKind::Plus) || check(TokenKind::Minus)) {
            const Token op = tokens_[pos_++];
            Expr inner = parse_unary();
            const std::size_t end = inner.end;
            return make_unary(op.text, std::move(inner), op.start, slice(op.start, end));
        }
        return parse_power();
    }

    Expr parse_power() {
        Expr left = parse_postfix();
        if (match(TokenKind::Caret)) {
            Expr right = parse_unary();
            const std::size_t start = left.start;
            const std::size_t end = right.end;
            left = make_binary("^", std::move(left), std::move(right), slice(start, end));
        }
        return left;
    }

    Expr parse_postfix() {
        Expr e = parse_primary();
        while (check(TokenKind::Bang)) {
            const Token bang = tokens_[pos_++];
            const std::size_t start = e.start;
            e = make_fact(std::move(e), bang.end, slice(start, bang.end));
        }
        return e;
    }

    Expr parse_primary() {
        if (check(TokenKind::Number)) {
            const Token t = tokens_[pos_++];
            return make_number(t.number, t.start, t.end, t.text);
        }
        if (check(TokenKind::Ident)) {
            const Token id = tokens_[pos_++];
            if (match(TokenKind::LParen)) {
                std::vector<Expr> args;
                if (!check(TokenKind::RParen)) {
                    args.push_back(parse_assign());
                    while (match(TokenKind::Comma)) args.push_back(parse_assign());
                }
                const Token rp = consume(TokenKind::RParen, "expected ')' after arguments");
                return make_call(id.text, std::move(args), id.start, rp.end, slice(id.start, rp.end));
            }
            return make_var(id.text, id.start, id.end, id.text);
        }
        if (match(TokenKind::LParen)) {
            const std::size_t start = tokens_[pos_ - 1].start;
            Expr inner = parse_assign();
            const Token rp = consume(TokenKind::RParen, "expected ')'");
            inner.start = start;
            inner.end = rp.end;
            inner.source = slice(start, rp.end);
            return inner;
        }
        if (check(TokenKind::End)) throw CalcError("unexpected end of expression");
        throw CalcError("unexpected token '" + peek().text + "'");
    }
};

inline Expr parse_expr(const std::string& src) {
    return Parser(src).parse();
}
