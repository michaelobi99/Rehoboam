#pragma once
#include <vector>
#include <cmath>
#include <tuple>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <string>



//...........................................................................................................................................................
//Mean and standard deviation
float mean(const std::vector<int>& scores) {
	if (scores.empty()) return 0;
	float result{ 0. };
	int size = scores.size();
	for (unsigned i{ 0 }; i < size; ++i) {
		result += scores[i];
	}
	result /= float(size);
	return result;
}


float standard_deviation(const std::vector<int>& scores, float mean) {
	if (scores.size() < 2) {
		return 0;
	}
	float sum_squared_diff{ 0.0 };
	int size = scores.size();
	for (const auto& elem : scores) {
		sum_squared_diff += (elem - mean) * (elem - mean);
	}
	sum_squared_diff /= float(size - 1);
	float stddev = std::sqrt(sum_squared_diff);
	return stddev;
}

float pooled_standard_deviation(
	const std::vector<int>& scores1,
	const std::vector<int>& scores2,
	float weight1 = 0.6f,
	float weight2 = 0.4f) {

	if (scores1.size() < 2 && scores2.size() < 2) return 0.0f;

	float mean1 = mean(scores1);
	float mean2 = mean(scores2);

	// Variance for each group
	float var1 = 0.0f;
	if (scores1.size() >= 2) {
		float std1 = standard_deviation(scores1, mean1);
		var1 = std1 * std1;
	}

	float var2 = 0.0f;
	if (scores2.size() >= 2) {
		float std2 = standard_deviation(scores2, mean2);
		var2 = std2 * std2;
	}

	// Weighted mean (for between-group variance)
	float combined_mean = weight1 * mean1 + weight2 * mean2;

	// Pooled variance
	float within_variance = weight1 * var1 + weight2 * var2;

	// Between-group variance
	float between_variance = weight1 * (mean1 - combined_mean) * (mean1 - combined_mean) +
		weight2 * (mean2 - combined_mean) * (mean2 - combined_mean);

	float total_variance = within_variance + between_variance;

	return std::sqrt(total_variance);
}
//............................................................................................................................................................




//............................................................................................................................................................
//Exponential smoothing
float exponential_smoothing(const std::vector<int>& scores, double alpha) {
	if (scores.empty()) return 0;
	if (scores.size() < 2) return scores[0];

	int N = scores.size();
	int k = std::min(5, N);
	double init = 0;
	for (int i = N - 1; i >= N - k; --i)
		init += scores[i];
	init /= (double)k;

	double smoothed = init;

	for (int i = N - 2; i >= 0; --i) {
		smoothed = alpha * scores[i] + (1 - alpha) * smoothed;
	}
	return smoothed;
}


auto double_exponential_smoothing(const std::vector<int>& vec) -> double {
	const double alpha = 0.35;  // level smoothing
	const double beta = 0.15; // trend smoothing

	if (vec.size() == 0) return 0.0;
	if (vec.size() < 2) return vec[0];

	double level = vec[0];
	double trend = vec[1] - vec[0];

	for (size_t i = 1; i < vec.size(); ++i) {
		double prev_level = level;
		level = alpha * vec[i] + (1.0 - alpha) * (level + trend);
		trend = beta * (level - prev_level) + (1.0 - beta) * trend;
	}

	// 1-step-ahead forecast
	int h = 1;
	return level + h * trend;
}

std::vector<int> moving_median_smoother(std::vector<int> const& scores, int window = 3) {
	size_t length = scores.size() - window + 1;
	std::vector<int> smoothed_scores(length);
	size_t mid = window / 2;
	for (int i{ 0 }; i < length; ++i) {
		std::vector<int> window_data(scores.begin() + i, scores.begin() + i + window);
		std::sort(window_data.begin(), window_data.end());
		smoothed_scores[i] = window_data[mid];
	}
	return smoothed_scores;
}
//......................................................................................................................................................


//.........................................................................................................................................................
//Z-Distribution
std::tuple<float, float> z_dist(size_t n, float mean, float stddev, double z_score = 1.96) {
	// Calculate standard error
	double std_error = stddev / std::sqrt(n);
	// Calculate range
	float low = mean - (z_score * std_error);
	float high = mean + (z_score * std_error);
	return std::tuple{ low, high };
}
//.........................................................................................................................................................

double skew(const std::vector<int>& array) {
	double avg = mean(array);
	size_t N = array.size();
	double result = 0.0;
	for (size_t i{ 0 }; i < N; ++i) {
		result += (array[i] - avg) * (array[i] - avg) * (array[i] - avg);
	}
	result /= (double)N;
	double var = std::pow(standard_deviation(array, avg), 2);
	var = std::pow(var, 1.5);
	if (var < 1e-9) return 0.0;
	result /= var;
	return result;
}

double kurtosis(const std::vector<int>& array) {
	double avg = mean(array);
	size_t N = array.size();

	if (N < 4) return 0.0;

	double result = 0.0;

	for (size_t i = 0; i < N; ++i) {
		double diff = array[i] - avg;
		result += diff * diff * diff * diff; // (x-μ)^4
	}
	result /= (double)N;

	double var = std::pow(standard_deviation(array, avg), 2);
	if (var < 1e-9) return 0.0;

	result /= (var * var); // divide by σ^4
	result -= 3.0;         // excess kurtosis (normal → 0)

	return result;
}

float predict_next_score(const std::vector<int>& scores) {
	return 0.5 * mean(scores) + 0.5 * exponential_smoothing(scores, 0.25);
}

//..........................................................................................................................................................
//T-Distribution

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

std::tuple<float, float> t_dist(size_t n, float mean, float stddev, float confidence_level = 0.95f) {
	if (n < 2) return { mean, mean };

	unsigned degrees_of_freedom = (n - 1);
	if (degrees_of_freedom > 29) degrees_of_freedom = 29;

	std::string string = std::to_string(confidence_level);
	std::string conf_as_str = string + ((string.size() >= 5) ? "" : std::string(5 - string.size(), '0'));
	float critical_value = t_table[degrees_of_freedom][getTailProbabilityIndex(conf_as_str)];

	float se_mean = stddev / std::sqrt(static_cast<float>(n));
	float margin_of_error = critical_value * se_mean;

	float low = mean - margin_of_error;
	float high = mean + margin_of_error;

	return { low, high };
}

//.......................................................................................................................................................

//Autoregressive model
double dot(const std::vector<double>& a, const std::vector<double>& b)
{
	double s = 0;
	for (size_t i = 0; i < a.size(); ++i)
		s += a[i] * b[i];
	return s;
}

std::vector<double> solve_normal_equations(
	const std::vector<std::vector<double>>& X,
	const std::vector<double>& y)
{
	int p = X[0].size();

	std::vector<std::vector<double>> XtX(p, std::vector<double>(p, 0));
	std::vector<double> Xty(p, 0);

	for (size_t i = 0; i < X.size(); ++i)
	{
		for (int j = 0; j < p; ++j)
		{
			Xty[j] += X[i][j] * y[i];
			for (int k = 0; k < p; ++k)
				XtX[j][k] += X[i][j] * X[i][k];
		}
	}

	// simple Gauss elimination
	for (int i = 0; i < p; ++i)
	{
		double pivot = XtX[i][i];
		for (int j = i; j < p; ++j)
			XtX[i][j] /= pivot;

		Xty[i] /= pivot;

		for (int k = 0; k < p; ++k)
		{
			if (k == i) continue;

			double factor = XtX[k][i];
			for (int j = i; j < p; ++j)
				XtX[k][j] -= factor * XtX[i][j];

			Xty[k] -= factor * Xty[i];
		}
	}

	return Xty;
}

double AR_forecast(const std::vector<double>& x, int p)
{
	int n = x.size();
	if (n <= p) return x.back();

	std::vector<std::vector<double>> X;
	std::vector<double> y;

	for (int t = p; t < n; ++t)
	{
		std::vector<double> row;

		for (int j = 1; j <= p; ++j)
			row.push_back(x[t - j]);

		X.push_back(row);
		y.push_back(x[t]);
	}

	std::vector<double> phi = solve_normal_equations(X, y);

	std::vector<double> last(p);
	for (int j = 1; j <= p; ++j)
		last[j - 1] = x[n - j];

	return dot(phi, last);
}

//ARIMA
std::vector<double> difference(const std::vector<double>& x, int d)
{
	std::vector<double> diff(x.begin(), x.end());

	for (int k = 0; k < d; ++k)
	{
		std::vector<double> temp;

		for (size_t i = 1; i < diff.size(); ++i)
			temp.push_back(diff[i] - diff[i - 1]);

		diff = temp;
	}

	return diff;
}

double ARIMA_forecast(const std::vector<double>& x, int p, int d)
{
	if (x.size() <= p + d)
		return x.back();

	std::vector<double> diff = difference(x, d);

	std::vector<double> diff_vector(diff.begin(), diff.end());

	double forecast_diff = AR_forecast(diff_vector, p);

	double result = x.back();

	for (int i = 0; i < d; ++i)
		result += forecast_diff;

	return result;
}