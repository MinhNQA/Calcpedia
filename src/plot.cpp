#include "plot.hpp"

#include "error.hpp"
#include "fmt.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace {

constexpr char kMarks[] = {'*', '+', 'o', '#', '@', 'x', '%'};

int clampi(int v, int lo, int hi) {
    return std::max(lo, std::min(hi, v));
}

} // namespace

std::string plot_ascii(Calculator& calc, const PlotSpec& spec) {
    if (spec.exprs.empty()) throw CalcError("plot needs at least one expression");
    if (!(spec.xmax > spec.xmin)) throw CalcError("plot range needs xmax > xmin");
    if (spec.width < 20 || spec.height < 7) throw CalcError("plot is too small");

    const int w = spec.width;
    const int h = spec.height;
    const double dx = (spec.xmax - spec.xmin) / static_cast<double>(w - 1);

    std::vector<std::vector<double>> ys(spec.exprs.size());
    double ymin = 0;
    double ymax = 0;
    bool any = false;

    for (std::size_t f = 0; f < spec.exprs.size(); ++f) {
        ys[f].resize(static_cast<std::size_t>(w));
        for (int c = 0; c < w; ++c) {
            const double x = spec.xmin + dx * c;
            double y = 0;
            try {
                y = calc.evaluate_with_x(spec.exprs[f], x);
                if (!std::isfinite(y)) y = std::numeric_limits<double>::quiet_NaN();
            } catch (...) {
                y = std::numeric_limits<double>::quiet_NaN();
            }
            ys[f][static_cast<std::size_t>(c)] = y;
            if (std::isfinite(y)) {
                if (!any) {
                    ymin = ymax = y;
                    any = true;
                } else {
                    ymin = std::min(ymin, y);
                    ymax = std::max(ymax, y);
                }
            }
        }
    }

    if (!any) throw CalcError("plot produced no finite values");
    if (ymax == ymin) {
        ymax += 1;
        ymin -= 1;
    }
    const double pad = 0.05 * (ymax - ymin);
    ymax += pad;
    ymin -= pad;

    std::vector<std::string> grid(static_cast<std::size_t>(h), std::string(static_cast<std::size_t>(w), ' '));

    auto col_of_x = [&](double x) {
        return clampi(static_cast<int>(std::llround((x - spec.xmin) / (spec.xmax - spec.xmin) * (w - 1))),
                      0, w - 1);
    };
    auto row_of_y = [&](double y) {
        const double t = (y - ymin) / (ymax - ymin);
        return clampi(static_cast<int>(std::llround((1.0 - t) * (h - 1))), 0, h - 1);
    };

    const bool x_axis = spec.xmin <= 0 && spec.xmax >= 0;
    const bool y_axis = ymin <= 0 && ymax >= 0;
    const int x_axis_row = y_axis ? row_of_y(0) : h - 1;
    const int y_axis_col = x_axis ? col_of_x(0) : 0;

    for (int c = 0; c < w; ++c) grid[static_cast<std::size_t>(x_axis_row)][static_cast<std::size_t>(c)] = '-';
    for (int r = 0; r < h; ++r) grid[static_cast<std::size_t>(r)][static_cast<std::size_t>(y_axis_col)] = '|';
    grid[static_cast<std::size_t>(x_axis_row)][static_cast<std::size_t>(y_axis_col)] = '+';

    for (std::size_t f = 0; f < spec.exprs.size(); ++f) {
        const char mark = kMarks[f % (sizeof(kMarks) - 1)];
        for (int c = 0; c < w; ++c) {
            const double y = ys[f][static_cast<std::size_t>(c)];
            if (!std::isfinite(y)) continue;
            const int r = row_of_y(y);
            grid[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] = mark;
        }
    }

    std::ostringstream out;
    out << "x in [" << format_number(spec.xmin) << ", " << format_number(spec.xmax) << "],  y in ["
        << format_number(ymin) << ", " << format_number(ymax) << "]\n";
    for (std::size_t f = 0; f < spec.exprs.size(); ++f) {
        const char mark = kMarks[f % (sizeof(kMarks) - 1)];
        const std::string& label = f < spec.labels.size() ? spec.labels[f] : spec.exprs[f].source;
        out << "  " << mark << "  " << label << "\n";
    }
    out << "\n";
    out << " " << format_number(ymax) << "\n";
    for (int r = 0; r < h; ++r) {
        out << "|" << grid[static_cast<std::size_t>(r)] << "|\n";
    }
    out << " " << format_number(ymin) << "\n";
    out << format_number(spec.xmin);
    {
        const std::string left = format_number(spec.xmin);
        const std::string right = format_number(spec.xmax);
        const int gap = w + 1 - static_cast<int>(left.size());
        if (gap > static_cast<int>(right.size())) {
            out << std::string(static_cast<std::size_t>(gap - static_cast<int>(right.size())), ' ') << right;
        } else {
            out << "  " << right;
        }
    }
    out << "\n";
    return out.str();
}

std::string table_values(Calculator& calc, const Expr& expr, const std::string& label, double xmin,
                         double xmax, double step) {
    if (!(xmax > xmin)) throw CalcError("table range needs xmax > xmin");
    if (!(step > 0)) throw CalcError("table step must be positive");

    std::ostringstream out;
    out << std::left << std::setw(16) << "x" << label << "\n";
    out << std::string(16 + std::max<std::size_t>(label.size(), 8), '-') << "\n";

    int rows = 0;
    for (double x = xmin; x <= xmax + step * 0.5 && rows < 200; x += step, ++rows) {
        std::string ytext;
        try {
            const double y = calc.evaluate_with_x(expr, x);
            ytext = format_number(y);
        } catch (const std::exception& ex) {
            ytext = std::string("error: ") + ex.what();
        }
        out << std::left << std::setw(16) << format_number(x) << ytext << "\n";
    }
    return out.str();
}
