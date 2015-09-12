#ifndef QUATERNION_H
#define QUATERNION_H

#include <string>
#include <sstream>

struct Quaternion {
    double w;
    double x;
    double y;
    double z;

    Quaternion(): w(0), x(0), y(0), z(0) {}

    std::string to_string() {
        std::stringstream stream;
        stream << "(" << w << ", " << x << ", " << y << ", " << z << ")";
        return stream.str();
    }
};

#endif