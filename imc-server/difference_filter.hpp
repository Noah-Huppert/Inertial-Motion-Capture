#ifndef DIFFERENCE_FILTER_H
#define DIFFERENCE_FILTER_H

#include <iostream>
#include <math.h>
#include <deque>

#include "log.h"

class DifferenceFilter {
public:
    double zero_value = 0;
    double difference_threshold = 0.1;

    DifferenceFilter(int list_size): list_size(list_size) {}

    double value(double value);

private:
    int list_size;
    std::deque<double> list;
};

#endif