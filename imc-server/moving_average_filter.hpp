#ifndef MOVING_AVERAGE_FILTER_H
#define MOVING_AVERAGE_FILTER_H

#include <deque>

class MovingAverageFilter {
public:
    MovingAverageFilter(int list_size): list_size(list_size) {}

    void add(double value);
    double average();

private:
    bool average_calculated = false;
    double calculated_average = 0;

    int list_size;
    std::deque<double> list;
};

#endif