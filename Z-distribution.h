#pragma once

#include <cmath>
#include <tuple>
#include <vector>



std::tuple<float, float> z_dist(size_t n, float mean, float stddev, double z_score = 1.96) {
    // Calculate standard error
    double std_error = stddev / std::sqrt(n);
    // Calculate range
    float low = mean - (z_score * std_error);
    float high = mean + (z_score * std_error);
    return std::tuple{ low, high };
}