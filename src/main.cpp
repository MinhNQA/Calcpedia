#include "error.hpp"
#include "eval.hpp"
#include "explain.hpp"
#include "fmt.hpp"
#include "handbook.hpp"
#include "repl.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace {

void print_usage(std::ostream& out) {
    out << "Calcpedia — terminal calculator with a math handbook\n"
           "\n"
           "usage:\n"
           "  calcpedia                 interactive prompt\n"
           "  calcpedia -e <expr>       evaluate one expression\n"
           "  calcpedia -x <expr>       explain one expression\n"
           "  calcpedia -c <line>       run one REPL command (plot, help, …)\n"
           "  calcpedia --self-test     run built-in checks\n"
           "  calcpedia -h              this help\n";
}

int self_test() {
    Calculator c;
    int failed = 0;
    auto expect = [&](const char* expr, double want) {
        try {
            const double got = c.evaluate(expr).value;
            if (std::fabs(got - want) > 1e-8 && !(want == 0 && std::fabs(got) < 1e-8)) {
                std::cerr << "FAIL " << expr << "  got " << got << " want " << want << "\n";
                ++failed;
            }
        } catch (const std::exception& ex) {
            std::cerr << "FAIL " << expr << "  exception: " << ex.what() << "\n";
            ++failed;
        }
    };

    expect("2+3*4", 14);
    expect("(1+2)^3", 27);
    expect("5!", 120);
    expect("2^3!", 64);
    expect("-2^2", -4);
    expect("2^-3", 0.125);
    expect("nCr(10,3)", 120);
    expect("nPr(5,2)", 20);
    expect("gcd(24,18)", 6);
    expect("lcm(4,6)", 12);
    expect("sqrt(16)/2", 2);
    expect("abs(-7)", 7);
    expect("hypot(3,4)", 5);
    expect("log(1000)", 3);
    expect("log2(8)", 3);
    c.set_angle_mode(AngleMode::Degrees);
    expect("sin(90)", 1);
    expect("cos(180)", -1);
    c.set_angle_mode(AngleMode::Radians);
    expect("sin(pi/2)", 1);
    expect("gamma(5)", 24);
    expect("phi", 1.618033988749895);
    c.evaluate("a = 5");
    expect("a + 2", 7);
    expect("ans", 7);

    Repl repl;
    const std::string plot = repl.handle_line("plot sin(x)/x -10 10");
    if (plot.find('*') == std::string::npos) {
        std::cerr << "FAIL plot produced no curve\n";
        ++failed;
    }
    const std::string ex = repl.handle_line("explain 5! + sqrt(16)/2");
    if (ex.find("result:") == std::string::npos || ex.find("120") == std::string::npos) {
        std::cerr << "FAIL explain\n" << ex;
        ++failed;
    }
    const std::string help = repl.handle_line("help gamma");
    if (help.find("Gamma") == std::string::npos && help.find("gamma") == std::string::npos) {
        std::cerr << "FAIL help gamma\n";
        ++failed;
    }
    const std::string search = repl.handle_line("search derivative");
    if (search.find("derivative") == std::string::npos) {
        std::cerr << "FAIL search derivative\n";
        ++failed;
    }

    if (failed == 0) {
        std::cout << "self-test: all checks passed\n";
        return 0;
    }
    std::cout << "self-test: " << failed << " failed\n";
    return 1;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc == 1) {
            Repl repl;
            const bool interactive = true;
            return repl.run(std::cin, std::cout, interactive);
        }

        const std::string a1 = argv[1];
        if (a1 == "-h" || a1 == "--help") {
            print_usage(std::cout);
            return 0;
        }
        if (a1 == "--self-test") return self_test();

        auto join_from = [&](int i) {
            std::string s;
            for (; i < argc; ++i) {
                if (!s.empty()) s.push_back(' ');
                s += argv[i];
            }
            return s;
        };

        if ((a1 == "-e" || a1 == "--eval") && argc >= 3) {
            Calculator c;
            std::cout << format_number(c.evaluate(join_from(2)).value) << "\n";
            return 0;
        }
        if ((a1 == "-x" || a1 == "--explain") && argc >= 3) {
            Calculator c;
            Handbook book;
            std::cout << explain_expression(c, book, join_from(2));
            return 0;
        }
        if ((a1 == "-c" || a1 == "--cmd") && argc >= 3) {
            Repl repl;
            const std::string out = repl.handle_line(join_from(2));
            std::cout << out;
            if (!out.empty() && out.back() != '\n') std::cout << "\n";
            return 0;
        }

        print_usage(std::cerr);
        return 2;
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << "\n";
        return 1;
    }
}
