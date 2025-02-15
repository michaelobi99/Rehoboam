#pragma once

#include <cmath>
#include <vector>

struct ZRange {
    double low;
    double high;
};

ZRange z_dist(size_t n, float mean, float stddev, double z_score = 1.96) {
    // Calculate standard error
    double std_error = stddev / std::sqrt(n);
    // Calculate range
    ZRange range;
    range.low = mean - (z_score * std_error);
    range.high = mean + (z_score * std_error);
    return range;
}