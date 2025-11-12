#pragma once
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>

enum class InterpolationMode { Nearest, Bilinear, Bicubic };

class Map2D {
  public:
    Map2D(){};
    Map2D(const std::string& filename){
        loadFromFile(filename);
    }
    std::vector<float> xAxis;
    std::vector<float> yAxis;
    std::vector<std::vector<float>> zData; // z[y][x]

    std::string xString, yString, zString;
    InterpolationMode currentInterpolation = InterpolationMode::Bilinear;

    // Add/edit/remove ---------------------------------

    void addPoint(float x, float y, float z) {
        // Find or insert X and Y indices
        auto xi = std::lower_bound(xAxis.begin(), xAxis.end(), x);
        auto yi = std::lower_bound(yAxis.begin(), yAxis.end(), y);

        size_t xIndex = xi - xAxis.begin();
        size_t yIndex = yi - yAxis.begin();

        bool newX = (xi == xAxis.end() || *xi != x);
        bool newY = (yi == yAxis.end() || *yi != y);

        // Expand axes if necessary
        if (newX) {
            xAxis.insert(xi, x);
            for (auto &row : zData)
                row.insert(row.begin() + xIndex, 0.0f);
        }

        if (newY) {
            yAxis.insert(yi, y);
            zData.insert(zData.begin() + yIndex, std::vector<float>(xAxis.size(), 0.0f));
        }

        zData[yIndex][xIndex] = z;
    }

    void editPoint(float x, float y, float newZ) {
        auto xi = std::find(xAxis.begin(), xAxis.end(), x);
        auto yi = std::find(yAxis.begin(), yAxis.end(), y);
        if (xi == xAxis.end() || yi == yAxis.end())
            throw std::runtime_error("Point not found");
        zData[yi - yAxis.begin()][xi - xAxis.begin()] = newZ;
    }

    void removePoint(float x, float y) {
        auto xi = std::find(xAxis.begin(), xAxis.end(), x);
        auto yi = std::find(yAxis.begin(), yAxis.end(), y);
        if (xi == xAxis.end() || yi == yAxis.end())
            throw std::runtime_error("Point not found");

        size_t xIndex = xi - xAxis.begin();
        size_t yIndex = yi - yAxis.begin();

        zData[yIndex][xIndex] = NAN; // mark deleted, keep structure
    }

    bool loadFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + filename);

        xAxis.clear();
        yAxis.clear();
        zData.clear();

        std::string line;
        bool headerRead = false;
        bool xLineRead = false;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);

            // Read header line: e.g. "rpm load engine_1.setAmplitude"
            if (!headerRead) {
                iss >> xString >> yString >> zString;
                headerRead = true;
                continue;
            }

            // Read X axis line starting with '\'
            if (!xLineRead && line.find('\\') != std::string::npos) {
                char slash;
                iss >> slash;
                float xVal;
                while (iss >> xVal)
                    xAxis.push_back(xVal);
                xLineRead = true;
                continue;
            }

            // Read Y and Z rows
            if (xLineRead) {
                float yVal;
                iss >> yVal;
                yAxis.push_back(yVal);

                std::vector<float> row;
                float zVal;
                while (iss >> zVal)
                    row.push_back(zVal);

                // Ensure row size matches X axis
                if (row.size() != xAxis.size())
                    throw std::runtime_error("Row length does not match X axis count at y=" + std::to_string(yVal));

                zData.push_back(row);
            }
        }

        if (!headerRead || !xLineRead)
            throw std::runtime_error("Malformed map file: missing header or X axis line");

        return true;
    }

    // Print --------------------------------------------

    void print() const {
        std::cout << " X: " << xString << " Y: " << yString << "Z (out): " << zString << "\n";
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "═════════╝";
        bool x_color = true;
        for (float x : xAxis) {
            x_color = !x_color;
            std::cout << (x_color ? "\033[44;1m" : "\033[40;1m") << std::setw(10) << x << "\033[0m";
        }
        std::cout << "\n";

        bool y_color = true;
        for (size_t i = 0; i < yAxis.size(); ++i) {
            y_color = !y_color;
            std::cout << (y_color ? "\033[44;1m" : "\033[40;1m") << std::setw(10) << yAxis[i] << "\033[0m";
            for (float z : zData[i])
                std::cout << std::setw(10) << z;
            std::cout << "\n";
        }
    }

    // Query --------------------------------------------

    float getValue(float x, float y) const {
        if (xAxis.empty() || yAxis.empty() || zData.empty())
            throw std::runtime_error("Map is empty");

        // Clamp inside bounds
        x = std::clamp(x, xAxis.front(), xAxis.back());
        y = std::clamp(y, yAxis.front(), yAxis.back());

        // Find grid indices
        size_t xi = std::upper_bound(xAxis.begin(), xAxis.end(), x) - xAxis.begin() - 1;
        size_t yi = std::upper_bound(yAxis.begin(), yAxis.end(), y) - yAxis.begin() - 1;

        xi = std::min(xi, xAxis.size() - 2);
        yi = std::min(yi, yAxis.size() - 2);

        float x0 = xAxis[xi], x1 = xAxis[xi + 1];
        float y0 = yAxis[yi], y1 = yAxis[yi + 1];
        float q11 = zData[yi][xi], q21 = zData[yi][xi + 1];
        float q12 = zData[yi + 1][xi], q22 = zData[yi + 1][xi + 1];

        if (currentInterpolation == InterpolationMode::Nearest)
            return nearest(x, y, xi, yi);

        if (currentInterpolation == InterpolationMode::Bilinear)
            return bilinear(x, y, x0, x1, y0, y1, q11, q21, q12, q22);

        if (currentInterpolation == InterpolationMode::Bicubic)
            return bicubic(x, y, xi, yi);

        return 0.0f;
    }

  private:
    float nearest(float x, float y, size_t xi, size_t yi) const {
        size_t nx = (x - xAxis[xi] > (xAxis[xi + 1] - x)) ? xi + 1 : xi;
        size_t ny = (y - yAxis[yi] > (yAxis[yi + 1] - y)) ? yi + 1 : yi;
        return zData[ny][nx];
    }

    float bilinear(float x, float y, float x0, float x1, float y0, float y1, float q11, float q21, float q12, float q22) const {
        float denom = (x1 - x0) * (y1 - y0);
        float fxy = (q11 * (x1 - x) * (y1 - y) + q21 * (x - x0) * (y1 - y) + q12 * (x1 - x) * (y - y0) + q22 * (x - x0) * (y - y0)) / denom;
        return fxy;
    }

    float bicubic(float x, float y, size_t xi, size_t yi) const {
        // Simple Catmull-Rom bicubic approximation
        auto get = [&](int yy, int xx) {
            yy = std::clamp(yy, 0, (int)yAxis.size() - 1);
            xx = std::clamp(xx, 0, (int)xAxis.size() - 1);
            return zData[yy][xx];
        };

        auto cubic = [&](float p0, float p1, float p2, float p3, float t) {
            float a0 = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
            float a1 = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
            float a2 = -0.5f * p0 + 0.5f * p2;
            float a3 = p1;
            return ((a0 * t + a1) * t + a2) * t + a3;
        };

        float tx = (x - xAxis[xi]) / (xAxis[xi + 1] - xAxis[xi]);
        float ty = (y - yAxis[yi]) / (yAxis[yi + 1] - yAxis[yi]);

        float col[4];
        for (int m = -1; m <= 2; ++m)
            col[m + 1] = cubic(get(yi - 1, xi + m), get(yi, xi + m), get(yi + 1, xi + m), get(yi + 2, xi + m), ty);

        return cubic(col[0], col[1], col[2], col[3], tx);
    }
};
