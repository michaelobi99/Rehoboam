#pragma once
#include <vector>

#define SMOOTHING_FACTOR 0.25

float exponential_smoothing(const std::vector<int>& scores) {
	std::vector<int> scores_reversed(std::rbegin(scores), std::rend(scores));
	if (scores.size() == 0) exit(1);
	int N = scores_reversed.size();
	float current_smoothed_value{ 0.0 }, current_value{ 0.0 }, previous_smoothed_value{ 0.0 };
	previous_smoothed_value = scores_reversed[0];
	for (int i = 1; i < N; ++i) {
		current_smoothed_value = SMOOTHING_FACTOR * scores[i] + (1 - SMOOTHING_FACTOR) * previous_smoothed_value;
		previous_smoothed_value = current_smoothed_value;
	}
	return current_smoothed_value;
}



float adaptive_exponential_smoothing(const std::vector<int>& scores, float initialAlpha = 0.5) {
    std::pair<float, float> alphaRange = { 0.1, 0.9 };
    // How much to adjust alpha when there's an error
    double adjustmentFactor = 0.1;

    // Handle edge cases
    if (scores.empty()) return 0.0;
    if (scores.size() == 1) return scores[0];

    float current_smoothed_value = scores[0];
    float alpha = initialAlpha;

    // Process each score
    for (int t = 1; t < scores.size(); ++t) {
        double error = scores[t] - current_smoothed_value;

        // Calculate new smoothed value
        current_smoothed_value = alpha * scores[t] + (1 - alpha) * current_smoothed_value;

        if (error != 0) {
            alpha += adjustmentFactor * (error > 0 ? 1 : -1);
            alpha = std::max(alphaRange.first, std::min(alphaRange.second, alpha));
        }
    }

    return current_smoothed_value;
}