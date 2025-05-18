#include "Damper.h"
Damper::Damper(int size) : maxSize(size) {}

// Averages float values
void Damper::addValue(double value) {
    values.push(value);
    if (values.size() > maxSize) {
        values.pop();
    }
}

double Damper::getAverage() const{
    if (values.empty()) {
        return 0.0; // Return 0 if queue is empty
    }

    double sum = 0.0;
    int count = 0;
    std::queue<double> temp = values; // Create a copy of the queue

    while (!temp.empty()) {
        sum += temp.front();
        temp.pop();
        count++;
    }

    return sum / count;
}