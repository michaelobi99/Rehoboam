#pragma once
#include <vector>

float simple_linear_regression(const std::vector<int>& scores) {
    if (scores.empty()) return 0;
    if (scores.size() < 2) return scores[0];

    int N = scores.size() - 1; // One less for pairs of (x,y)
    std::vector<int> x(scores.rbegin(), scores.rend() - 1); // previous scores
    std::vector<int> Y(scores.rbegin() + 1, scores.rend()); // next scores

    float sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    for (int i = 0; i < N; i++) {
        sum_x += x[i];
        sum_y += Y[i];
        sum_xy += x[i] * Y[i];
        sum_x2 += x[i] * x[i];
    }

    float denominator = (N * sum_x2 - sum_x * sum_x);
    if (std::abs(denominator) < 1e-6) { // If no clear trend, return last score
        return x[0];
    }

    float slope = (N * sum_xy - sum_x * sum_y) / denominator;
    float y_intercept = (sum_y - slope * sum_x) / N;

    // Use most recent score (Y[N-1] since vector is reversed) for prediction
    return (slope * Y[N-1]) + y_intercept;
}

//Account for home and away features
float multiple_linear_regression() {

}