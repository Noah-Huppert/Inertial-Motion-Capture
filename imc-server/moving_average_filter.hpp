#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <deque>

class MovingAverageFilter {
public:
    int size;

    MovingAverageFilter(int size): size(size) {}

    void add(double value);
    double average();

private:
    int effective_size();

    std::deque<double> list;
};

#endif