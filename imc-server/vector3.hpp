#ifndef VECTOR3_H
#define VECTOR3_H

#include <string>
#include <sstream>

struct Vector3 {
    double x;
    double y;
    double z;

    Vector3(): x(0), y(0), z(0) {}

    std::string to_space_delimited() {
        std::stringstream stream;
        stream << x << " " << y << " " << z;
        return stream.str();
    }
};

#endif