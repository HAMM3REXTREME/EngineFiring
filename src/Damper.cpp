#include "Damper.h"
Damper::Damper(int size) : maxSize(size) {}

void Damper::addValue(double value) {
    values.push(value);
    sum += value;

    if (values.size() > maxSize) {
        sum -= values.front();
        values.pop();
    }
}

double Damper::getAverage() const {
    if (values.empty()) return 0.0;
    return sum / values.size();
}