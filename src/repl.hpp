#pragma once

#include "eval.hpp"
#include "handbook.hpp"

#include <iosfwd>
#include <string>

class Repl {
public:
    Repl() = default;
    int run(std::istream& in, std::ostream& out, bool interactive);
    std::string handle_line(const std::string& line);

private:
    Calculator calc_;
    Handbook book_;
    std::string last_topic_;

    std::string cmd_help(const std::string& arg);
    std::string cmd_search(const std::string& arg);
    std::string cmd_try(const std::string& arg);
    std::string cmd_plot(const std::string& arg);
    std::string cmd_table(const std::string& arg);
    std::string cmd_vars() const;
};
