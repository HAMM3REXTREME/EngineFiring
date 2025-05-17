#pragma once

#include <queue>

class Damper {
  private:
    std::queue<double> values;
    int maxSize;
    double sum = 0.0;

  public:
    Damper(int size);

    void addValue(double value);

    double getAverage() const;
};
