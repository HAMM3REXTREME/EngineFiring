// Averages float values
#pragma once

class Damper {
    private:
     std::queue<double> values;
     int maxSize;
 
    public:
     Damper(int size) : maxSize(size) {}
 
     void addValue(double value);
 
     double getAverage();
 };
 