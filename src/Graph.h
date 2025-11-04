#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

class Graph {
public:
    enum class InterpType {
        Linear,
        Quadratic,
        Cubic,
        CatmullRom
    };

private:
    std::vector<std::pair<double, double>> data;

    // Helper: safely access data points with clamping at edges
    const std::pair<double,double>& safePoint(int idx) const {
        if (idx < 0) return data.front();
        if (idx >= (int)data.size()) return data.back();
        return data[idx];
    }

public:
    // Add one point (keeps sorted)
    void addPoint(double x, double y) {
        data.emplace_back(x, y);
        std::sort(data.begin(), data.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });
    }

    // Replace entire dataset
    void setData(std::vector<std::pair<double, double>> newData) {
        data = std::move(newData);
        std::sort(data.begin(), data.end(),
                  [](const auto &a, const auto &b) { return a.first < b.first; });
    }

    void clear() { data.clear(); }
    size_t size() const { return data.size(); }

    // Load from file: supports comments (#) and blank lines
    bool loadFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open file '" << filename << "'\n";
            return false;
        }

        std::vector<std::pair<double, double>> temp;
        std::string line;
        double x, y;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            if (!(iss >> x >> y)) {
                std::cerr << "Warning: Skipping malformed line: " << line << "\n";
                continue;
            }
            temp.emplace_back(x, y);
        }

        if (temp.empty()) {
            std::cerr << "Error: No valid data points found in file.\n";
            return false;
        }

        setData(std::move(temp));
        return true;
    }

    // Base linear interpolation
    double getValue(double x) const {
        return getValue(x, InterpType::Linear);
    }

    // Interpolated or extrapolated value with mode
    double getValue(double x, InterpType type) const {
        if (data.empty())
            throw std::runtime_error("No data points available.");

        if (x <= data.front().first) return data.front().second;
        if (x >= data.back().first)  return data.back().second;

        // Binary search for position
        auto it = std::lower_bound(data.begin(), data.end(), x,
                                   [](const auto &p, double value) { return p.first < value; });
        size_t i = std::distance(data.begin(), it);
        if (i == 0) i = 1;

        double x1 = data[i-1].first, y1 = data[i-1].second;
        double x2 = data[i].first,   y2 = data[i].second;
        double t = (x - x1) / (x2 - x1);

        switch (type) {
            case InterpType::Linear:
                return y1 + t * (y2 - y1);

            case InterpType::Quadratic: {
                // simple quadratic using one point before and after if possible
                const auto &p0 = safePoint((int)i - 2);
                const auto &p1 = data[i-1];
                const auto &p2 = data[i];
                double x0 = p0.first, y0 = p0.second;
                double denom = (x0 - x1) * (x0 - x2) * (x1 - x2);
                if (denom == 0.0) return y1 + t*(y2 - y1);
                double a = (x*(x - x1)*(x - x2)) / denom; // this term doesn’t belong here
                // Proper Lagrange form:
                double L0 = (x - x1)*(x - x2) / ((x0 - x1)*(x0 - x2));
                double L1 = (x - x0)*(x - x2) / ((x1 - x0)*(x1 - x2));
                double L2 = (x - x0)*(x - x1) / ((x2 - x0)*(x2 - x1));
                return L0*y0 + L1*y1 + L2*y2;
            }

            case InterpType::Cubic: {
                // Hermite cubic interpolation (smooth first derivative)
                const auto &p0 = safePoint((int)i - 2);
                const auto &p1 = data[i-1];
                const auto &p2 = data[i];
                const auto &p3 = safePoint((int)i + 1);

                double m1 = (p2.second - p0.second) / (p2.first - p0.first);
                double m2 = (p3.second - p1.second) / (p3.first - p1.first);

                double t2 = t * t;
                double t3 = t2 * t;
                double h00 = 2*t3 - 3*t2 + 1;
                double h10 = t3 - 2*t2 + t;
                double h01 = -2*t3 + 3*t2;
                double h11 = t3 - t2;

                double dx = x2 - x1;
                return h00*y1 + h10*m1*dx + h01*y2 + h11*m2*dx;
            }

            case InterpType::CatmullRom: {
                const auto &p0 = safePoint((int)i - 2);
                const auto &p1 = data[i-1];
                const auto &p2 = data[i];
                const auto &p3 = safePoint((int)i + 1);

                double t2 = t * t;
                double t3 = t2 * t;
                return 0.5 * (
                    (2 * p1.second) +
                    (-p0.second + p2.second) * t +
                    (2*p0.second - 5*p1.second + 4*p2.second - p3.second) * t2 +
                    (-p0.second + 3*p1.second - 3*p2.second + p3.second) * t3
                );
            }
        }

        return y1; // fallback
    }

    void print() const {
        for (const auto &[x, y] : data)
            std::cout << "(" << x << ", " << y << ")\n";
    }
};
