#include "moving_average_filter.hpp"

void MovingAverageFilter::add(double value) {
    list.push_front(value);

    while(list.size() > list_size) {
        list.pop_back();
    }

    average_calculated = false;
}

double MovingAverageFilter::average() {
    if(!average_calculated) {
        double total = 0;

        for (int i = 0; i < list.size(); i++) {
            total += list[i];
        }

        calculated_average = total / list.size();
        average_calculated = true;
    }

    return calculated_average;
}