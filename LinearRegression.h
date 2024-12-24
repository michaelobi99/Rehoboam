#pragma once
#include <vector>

float simple_linear_regression(const std::vector<int>& scores) {
	int N = scores.size();
	std::vector<int> x(N);
	for (int i = N-1; i >= 0; --i) x[i] = i;
	float sum_x{ 0.0 }, sum_y{ 0.0 }, sum_xy{ 0.0 }, sum_x2{ 0.0 };
	for (int i = 0; i < N; i++) {
		sum_x += x[i];
		sum_y += scores[i];
		sum_xy += x[i] * scores[i];
		sum_x2 += x[i] * x[i];
	}

	double slope = (N * sum_xy - sum_x * sum_y) / (float)(N * sum_x2 - sum_x * sum_x);
	int next_game = N + 1;
	float y_intercept = (sum_y - slope * sum_x) / (float)N;
	return (slope * next_game) + y_intercept;
}


//Account for home and away features
float multiple_linear_regression() {

}