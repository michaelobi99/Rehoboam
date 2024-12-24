#pragma once
#include <vector>

#define SMOOTHING_FACTOR 0.3

float exponential_smoothing(const std::vector<int>& scores) {
	std::vector<int> scores_reversed(std::rbegin(scores), std::rend(scores));
	if (scores.size() == 0) exit(1);
	int N = scores_reversed.size();
	double current_smoothed_value{ 0.0 }, current_value{ 0.0 }, previous_smoothed_value{ 0.0 };
	previous_smoothed_value = scores_reversed[0];
	for (int i = 1; i < N; ++i) {
		current_smoothed_value = SMOOTHING_FACTOR * scores[i] + (1 - SMOOTHING_FACTOR) * previous_smoothed_value;
		previous_smoothed_value = current_smoothed_value;
	}
	return current_smoothed_value;
}