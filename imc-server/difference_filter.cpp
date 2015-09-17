#include "difference_filter.hpp"

double DifferenceFilter::value(double value) {
    if(abs(value) >= difference_threshold) {
        return value;
    } else {
        return zero_value;
    }
}