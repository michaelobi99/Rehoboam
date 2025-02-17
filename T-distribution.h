#pragma once
#include <vector>
#include <iomanip>
#include <sstream>
#include <string>

//t_table[degree_of_freedom][tail_probability]
float t_table[30][11] = {
	{0.000, 1.000, 1.376, 1.963, 3.078, 6.314, 12.71, 31.82, 63.66, 318.31, 636.62},
	{0.000, 0.816, 1.061, 1.386, 1.886, 2.920, 4.303, 6.965, 9.925, 22.327, 31.599},
	{0.000, 0.765, 0.978, 1.250, 1.638, 2.353, 3.182, 4.541, 5.841, 10.215, 12.924},
	{0.000, 0.741, 0.941, 1.190, 1.533, 2.132, 2.776, 3.747, 4.604, 7.173, 8.610},
	{0.000, 0.727, 0.920, 1.156, 1.476, 2.015, 2.571, 3.365, 4.032, 5.893, 6.869},
	{0.000, 0.718, 0.906, 1.134, 1.440, 1.943, 2.447, 3.143, 3.707, 5.208, 5.959},
	{0.000, 0.711, 0.896, 1.119, 1.415, 1.895, 2.365, 2.998, 3.499, 4.785, 5.408},
	{0.000, 0.706, 0.889, 1.108, 1.397, 1.860, 2.306, 2.896, 3.355, 4.501, 5.041},
	{0.000, 0.703, 0.883, 1.100, 1.383, 1.833, 2.262, 2.821, 3.250, 4.297, 4.781},
	{0.000, 0.700, 0.879, 1.093, 1.372, 1.812, 2.228, 2.764, 3.169, 4.144, 4.587},
	{0.000, 0.697, 0.876, 1.088, 1.363, 1.796, 2.201, 2.718, 3.106, 4.025, 4.437},
	{0.000, 0.695, 0.873, 1.083, 1.356, 1.782, 2.179, 2.681, 3.055, 3.930, 4.318},
	{0.000, 0.694, 0.870, 1.079, 1.350, 1.771, 2.160, 2.650, 3.012, 3.852, 4.221},
	{0.000, 0.692, 0.868, 1.076, 1.345, 1.761, 2.145, 2.624, 2.977, 3.787, 4.140},
	{0.000, 0.691, 0.866, 1.074, 1.341, 1.753, 2.131, 2.602, 2.947, 3.733, 4.073},
	{0.000, 0.690, 0.865, 1.071, 1.337, 1.746, 2.120, 2.583, 2.921, 3.686, 4.015},
	{0.000, 0.689, 0.863, 1.069, 1.333, 1.740, 2.110, 2.567, 2.898, 3.646, 3.965},
	{0.000, 0.688, 0.862, 1.067, 1.330, 1.734, 2.101, 2.552, 2.878, 3.610, 3.922},
	{0.000, 0.688, 0.861, 1.066, 1.328, 1.729, 2.093, 2.539, 2.861, 3.579, 3.883},
	{0.000, 0.687, 0.860, 1.064, 1.325, 1.725, 2.086, 2.528, 2.845, 3.552, 3.850},
	{0.000, 0.686, 0.859, 1.063, 1.323, 1.721, 2.080, 2.518, 2.831, 3.527, 3.819},
	{0.000, 0.686, 0.858, 1.061, 1.321, 1.717, 2.074, 2.508, 2.819, 3.505, 3.792},
	{0.000, 0.685, 0.858, 1.060, 1.319, 1.714, 2.069, 2.500, 2.807, 3.485, 3.768},
	{0.000, 0.685, 0.857, 1.059, 1.318, 1.711, 2.064, 2.492, 2.797, 3.467, 3.745},
	{0.000, 0.684, 0.856, 1.058, 1.316, 1.708, 2.060, 2.485, 2.787, 3.450, 3.725},
	{0.000, 0.684, 0.856, 1.058, 1.315, 1.706, 2.056, 2.479, 2.779, 3.435, 3.707},
	{0.000, 0.684, 0.855, 1.057, 1.314, 1.703, 2.052, 2.473, 2.771, 3.421, 3.690},
	{0.000, 0.683, 0.855, 1.056, 1.313, 1.701, 2.048, 2.467, 2.763, 3.408, 3.674},
	{0.000, 0.683, 0.854, 1.055, 1.311, 1.699, 2.045, 2.462, 2.756, 3.396, 3.659},
	{0.000, 0.683, 0.854, 1.055, 1.310, 1.697, 2.042, 2.457, 2.750, 3.385, 3.646}
};

int getTailProbabilityIndex(std::string confidence_level) {
	if (confidence_level.starts_with("0.0")) return 0;
	if (confidence_level.starts_with("0.5")) return 1;
	if (confidence_level.starts_with("0.6")) return 2;
	if (confidence_level.starts_with("0.7")) return 3;
	if (confidence_level.starts_with("0.8")) return 4;
	if (confidence_level.starts_with("0.90")) return 5;
	if (confidence_level.starts_with("0.95")) return 6;
	if (confidence_level.starts_with("0.98")) return 7;
	if (confidence_level.starts_with("0.99")) return 8;
	if (confidence_level.starts_with("0.998")) return 9;
	if (confidence_level.starts_with("0.999")) return 10;
	printf("Confidence level not recognized\n");
	exit(1);
}

std::tuple<float, float> t_dist(size_t n, float mean, float stddev, float confidence_level = 0.95) {
	unsigned degrees_of_freedom = (n - 1);
	if (degrees_of_freedom > 29) {
		printf("Error: Number of samples exceed optimal number for t-distribution");
		exit(1);
	}
	std::string string = std::to_string(confidence_level);
	std::string conf_as_str = string + ((string.size() >= 5) ? "" : std::string(5 - string.size(), '0'));
	float critical_value = t_table[degrees_of_freedom][getTailProbabilityIndex(conf_as_str)];
	float margin_of_error = stddev / (float)(sqrt(n));
	float low = mean - (critical_value * margin_of_error);
	float high = mean + (critical_value * margin_of_error);
	return std::tuple{ low, high };
}