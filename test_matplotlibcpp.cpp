#include "matplotlibcpp.h"

#include <cmath>

namespace plt = matplotlibcpp;

int main(int, char**)
{
    // test 1
    {
        plt::plot({1,3,2,4});
        plt::show();
    }

    // test 2
    {
        int n = 5000;
        std::vector<double> x(n), y(n), z(n), w(n, 2);
        for (int i = 0; i < n; ++i) {
            x.at(i) = i * i;
            y.at(i) = sin(2 * M_PI * i / 360.0);
            z.at(i) = log(i);
        }

        // Set the size of output image to 1200x780 pixels
        plt::figure_size(1200, 780);
        // Plot line from given x and y data. Color is selected automatically.
        plt::plot(x, y);
        // Plot a red dashed line from given x and y data.
        plt::plot(x, w, "r--");
        // Plot a line whose name will show up as "log(x)" in the legend.
        plt::named_plot("log(x)", x, z);
        // Set x-axis to interval [0, 1000000]
        plt::xlim(0, 1000 * 1000);
        // Add graph title
        plt::title("Sample figure");
        // Enable legend
        plt::legend();
        // Save the image (file format is determined by the extension)
        plt::save("./basic.png");
        // Show
        plt::show();
    }

    // test 3 : lambda formula
    {
        using namespace std;
        int n = 5000;
        vector<double> x(n), y(n);
        for (int i = 0; i < n; ++i) {
            double t = 2 * M_PI * i / n;
            x.at(i) = 16 * sin(t) * sin(t) * sin(t);
            y.at(i) = 13 * cos(t) - 5 * cos(2*t) - 2 * cos(3*t) - cos(4*t);
        }

        // plot() takes an arbitrary number of (x,y,format)-triples.
        // x must be iterable (that is, anything providing begin(x) and end(x)),
        // y must either be callable (providing operator() const) or iterable.
        plt::plot(
            x, y, "r--", // 1つ目の関数
            x, [](double d) { return 12.5 + abs(sin(d)); }, "k--" // 2つ目の関数
        );

        plt::show();
    }

    // test 4 : vector field
    {
        using namespace std;
        vector<int> x, y, u, v;
        for (int i = -5; i <= 5; ++i) {
            for (int j = -5; j <= 5; ++j) {
                x.push_back(i);   // 支点x
                u.push_back(-i);  // ベクトルの向きx
                y.push_back(j);   // 支点y
                v.push_back(-j);  // ベクトルの向きy
            }
        }

        plt::quiver(x, y, u, v);
        plt::show();
    }

    // test 5 : 3D
    {
        using namespace std;
        vector<vector<double>> x, y, z;
        for (double i = -5; i <= 5; i += 0.25) { // 列方向(x)
            vector<double> x_row, y_row, z_row;
            for (double j = -5; j <= 5; j += 0.25) { // 行方向(y)
                x_row.push_back(i);
                y_row.push_back(j);
                z_row.push_back(sin(hypot(i, j)));
            }
            x.push_back(x_row);
            y.push_back(y_row);
            z.push_back(z_row);
        }

        plt::plot_surface(x, y, z);
        plt::show();
    }
}