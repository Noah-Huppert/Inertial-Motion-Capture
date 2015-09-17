#ifndef DIFFERENCE_FILTER_H
#define DIFFERENCE_FILTER_H

#include <stdlib.h>
#include <deque>

class DifferenceFilter {
public:
    double zero_value = 0;
    double difference_threshold = 0.1;

    DifferenceFilter() {}

    double value(double value);
};

#endif