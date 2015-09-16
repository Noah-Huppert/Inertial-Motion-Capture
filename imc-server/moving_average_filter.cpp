#include "moving_average_filter.hpp"

void MovingAverageFilter::add(double value) {
    list.push_front(value);

    while(list.size() > size) {
        list.pop_back();
    }
}

double MovingAverageFilter::average() {
    float total = 0;

    for(int i = 0; i < effective_size(); i++) {
        total += list[i];
    }

    return total / effective_size();
}

int MovingAverageFilter::effective_size() {
    if(list.size() <= size) {
        return list.size();
    } else {
        return size;
    }
}