#pragma once

#include "error.hpp"

#include <cctype>
#include <string>
#include <vector>

enum class TokenKind {
    Number,
    Ident,
    Plus,
    Minus,
    Star,
    Slash,
    Percent,
    Caret,
    Bang,
    Equal,
    LParen,
    RParen,
    Comma,
    End
};

struct Token {
    TokenKind kind;
    std::string text;
    double number = 0;
    std::size_t start = 0;
    std::size_t end = 0;
};

class Lexer {
public:
    explicit Lexer(std::string src) : src_(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            Token t = next();
            tokens.push_back(t);
            if (t.kind == TokenKind::End) break;
        }
        return tokens;
    }

private:
    std::string src_;
    std::size_t i_ = 0;

    void skip_ws() {
        while (i_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[i_]))) ++i_;
    }

    Token make(TokenKind k, std::size_t start, std::size_t end, std::string text, double n = 0) {
        Token t;
        t.kind = k;
        t.start = start;
        t.end = end;
        t.text = std::move(text);
        t.number = n;
        return t;
    }

    Token next() {
        skip_ws();
        if (i_ >= src_.size()) return make(TokenKind::End, i_, i_, "");

        const std::size_t start = i_;
        const char c = src_[i_];

        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '.' && i_ + 1 < src_.size() && std::isdigit(static_cast<unsigned char>(src_[i_ + 1])))) {
            return number(start);
        }
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            ++i_;
            while (i_ < src_.size() &&
                   (std::isalnum(static_cast<unsigned char>(src_[i_])) || src_[i_] == '_')) {
                ++i_;
            }
            return make(TokenKind::Ident, start, i_, src_.substr(start, i_ - start));
        }

        ++i_;
        switch (c) {
            case '+': return make(TokenKind::Plus, start, i_, "+");
            case '-': return make(TokenKind::Minus, start, i_, "-");
            case '*': return make(TokenKind::Star, start, i_, "*");
            case '/': return make(TokenKind::Slash, start, i_, "/");
            case '%': return make(TokenKind::Percent, start, i_, "%");
            case '^': return make(TokenKind::Caret, start, i_, "^");
            case '!': return make(TokenKind::Bang, start, i_, "!");
            case '=': return make(TokenKind::Equal, start, i_, "=");
            case '(': return make(TokenKind::LParen, start, i_, "(");
            case ')': return make(TokenKind::RParen, start, i_, ")");
            case ',': return make(TokenKind::Comma, start, i_, ",");
            default:
                throw CalcError("unexpected character '" + std::string(1, c) + "'");
        }
    }

    Token number(std::size_t start) {
        while (i_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[i_]))) ++i_;
        if (i_ < src_.size() && src_[i_] == '.') {
            ++i_;
            while (i_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[i_]))) ++i_;
        }
        if (i_ < src_.size() && (src_[i_] == 'e' || src_[i_] == 'E')) {
            const std::size_t epos = i_;
            ++i_;
            if (i_ < src_.size() && (src_[i_] == '+' || src_[i_] == '-')) ++i_;
            if (i_ >= src_.size() || !std::isdigit(static_cast<unsigned char>(src_[i_]))) {
                i_ = epos;
            } else {
                while (i_ < src_.size() && std::isdigit(static_cast<unsigned char>(src_[i_]))) ++i_;
            }
        }
        const std::string text = src_.substr(start, i_ - start);
        try {
            return make(TokenKind::Number, start, i_, text, std::stod(text));
        } catch (...) {
            throw CalcError("invalid number '" + text + "'");
        }
    }
};
