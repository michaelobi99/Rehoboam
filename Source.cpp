#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <string>
#include <tuple>
#include <ranges>
#include "T-distribution.h"
#include "Z-distribution.h"
#include "ExponentialSmoothing.h"
#include "LinearRegression.h"
#include "ARIMA.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif // _MSC_VER


std::string trim(const std::string& str) {
	auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
		return std::isspace(ch);
	});

	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
		return std::isspace(ch);
	}).base();

	return (start < end) ? std::string(start, end) : "";
}

std::tuple<std::string, std::vector<int>, std::vector<int>> split_string(const char* str, size_t length) {
	std::istringstream stream(str);

	std::string name;
	std::getline(stream, name, ':');
	name = trim(name);

	std::string past_scores_str{ "" };
	std::getline(stream, past_scores_str, ':');
	past_scores_str = trim(past_scores_str);

	std::string h2h_scores_str{""};
	std::getline(stream, h2h_scores_str, ':');


	auto to_int_vec = [](std::string& str) {
		std::vector<int> vec;
		std::string s{ "" };
		for (int i = 0; i < str.size(); ++i) {
			if (isdigit(str[i]))
				s.push_back(str[i]);
			else {
				if (s.size() > 0) {
					vec.push_back(std::stoi(s));
					s.clear();
				}
			}
		}
		if (s.size() > 0) vec.push_back(std::stoi(s));
		return vec;
	};

	std::vector<int> past_scores = to_int_vec(past_scores_str);
	std::vector<int> h2h_scores = (h2h_scores_str.size() > 0) ? to_int_vec(h2h_scores_str) : std::vector<int>{};
	return std::tuple(name, past_scores, h2h_scores);
}


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
	if (scores.empty()) return 0;
	float sum_squared_diff{ 0.0 };
	int size = scores.size();
	for (unsigned i{ 0 }; i < size; ++i) {
		sum_squared_diff += std::pow((mean - scores[i]), 2);
	}
	float stddev = std::sqrt(sum_squared_diff / (float)size - 1);
	return stddev;
}

void get_recommendation(float pred_1, float pred_2, float pred_3, float low, float high) {
	const int max_pred_diff = 10;
	const int max_dist_range = 20;
	float avg_points = 0.3f * pred_1 + 0.4f * pred_2 + 0.3f * pred_3;
	bool close_prediction = std::abs(pred_1 - pred_2) < max_pred_diff && std::abs(pred_1 - pred_3) < max_pred_diff && std::abs(pred_2 - pred_3) < max_pred_diff;
	bool tight_range = (int)(high - low) <= max_dist_range;
	if (close_prediction && tight_range) {
		printf("RECOMMENDATION: Very high confidence - (close predictions, tight range) Expect about %.2f total points.\n\n", avg_points);
	}
	else if (close_prediction && !tight_range) {
		printf("RECOMMENDATION: High confidence - (close predictions, wide range) Expect about %.2f total points.\n\n", avg_points);
	}
	else if (!close_prediction && tight_range) {
		printf("RECOMMENDATION: High confidence - (wide predictions, tight range) Expect about %.2f total points.\n\n", avg_points);
	}
	else {
		printf("RECOMMENDATION: Low confidence - (wide predictions, wide range) Expect about %.2f total points.\n\n", avg_points);
	}
}


int main(int argc, char* argv[]) {
	//only basketball is currently supported.
	//Future sports include, Tennis, Baseball, Hockey and Football.-[;
	if (argc < 2) {
		printf("Usage: Rehoboam [filename]\n");
		exit(1);
	}
	const char* file_path = argv[1];
	std::string match_details;
	std::string match_string;
	std::string line;
	std::vector<int> scores;
	std::fstream file{ file_path, std::ios::in | std::ios::binary};
	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}
	std::ostringstream stream;

	float recommendation{ 0.0 };

	//This loop reads all non-empty lines in the file. Three lines in a single loop.
	while (!file.eof()) {
		//Read until a non-empty line is found
		while (std::getline(file, line) && line.size() <= 1) continue;
		if (file.eof()) break;

		auto [home_team_name, home_past_scores, home_h2h_scores] = split_string(line.c_str(), line.size());
		match_string += home_team_name;
		
		line.clear();
		while (std::getline(file, line) && line.size() <= 1) continue;

		auto [away_team_name, away_past_scores, away_h2h_scores] = split_string(line.c_str(), line.size());
		match_string += " vs " + away_team_name;
		
		line.clear();
		while (std::getline(file, match_details) && match_details.size() <= 1) continue;

		//..........................................................................................................................
		//Exponential Smoothing
		float home_exp_pred = exponential_smoothing(home_past_scores) * ((home_h2h_scores.size() == 0) ? 1.0 : 0.6) + mean(home_h2h_scores) * 0.4;
		float away_exp_pred = exponential_smoothing(away_past_scores) * ((away_h2h_scores.size() == 0) ? 1.0 : 0.6) + mean(away_h2h_scores) * 0.4;

		//Simple Linear Regression
		float home_regression_pred = simple_linear_regression(home_past_scores) * ((home_h2h_scores.size() == 0) ? 1.0 : 0.6) + mean(home_h2h_scores) * 0.4;
		float away_regression_pred = simple_linear_regression(away_past_scores) * ((away_h2h_scores.size() == 0) ? 1.0 : 0.6) + mean(away_h2h_scores) * 0.4;


		std::copy(home_h2h_scores.begin(), home_h2h_scores.end(), std::back_inserter(home_past_scores));
		std::copy(away_h2h_scores.begin(), away_h2h_scores.end(), std::back_inserter(away_past_scores));

		float home_mean = mean(home_past_scores);
		float home_stddev = standard_deviation(home_past_scores, home_mean);
		float away_mean = mean(away_past_scores);
		float away_stddev = standard_deviation(away_past_scores, away_mean);
		float mean_total = home_mean + away_mean;

		float home_lower{ 0 }, away_lower{ 0 };
		float home_upper{ 0 }, away_upper{ 0 };

		if (home_past_scores.size() > 30 || away_past_scores.size() > 30) {
			//Normal distribution calculation
			std::tie(home_lower, home_upper) = z_dist(home_past_scores.size(), home_mean, home_stddev);
			std::tie(away_lower, away_upper) = z_dist(away_past_scores.size(), away_mean, away_stddev);
		}
		else {
			//T-distribution calculation
			std::tie(home_lower, home_upper) = t_dist(home_past_scores.size(), home_mean, home_stddev);
			std::tie(away_lower, away_upper) = t_dist(away_past_scores.size(), away_mean, away_stddev);
		}
		

		//ARIMA
		/*double arima_home_pred = predictARIMA(home_score);
		double arima_away_pred = predictARIMA(away_score);*/

		//.......................................................................................................................

#ifdef _MSC_VER
		std::random_device rd{};
		auto mtgen = std::mt19937{ rd() };
		auto ud = std::uniform_int_distribution<int>{ 1, 7 };
		int random_number = ud(mtgen);
		int text_color = 0;
		switch (random_number) {
		case 1: text_color = 5;
			break;
		case 2: text_color = 6;
			break;
		case 3: text_color = 9;
			break;
		case 4: text_color = 10;
			break;
		case 5: text_color = 12;
			break;
		case 6: text_color = 13;
			break;
		case 7: text_color = 14;
			break;
		}
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, text_color | FOREGROUND_INTENSITY);
#endif // Change console color

		//Display results
		std::string design(match_details.size() + match_string.size() + 5, '.');
		std::cout << design<<"\n";
		std::cout << match_string <<" - " << match_details << "\n";
		std::cout << design << "\n";

		const char* str1 = "Likely Score Range";
		std::cout << std::setw(38) << std::right << str1 << "\n";
		
		//Home
		stream << "Home: " << home_mean;
		printf("%-20s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_lower << " - " << home_upper << ")";
		printf("%s\n", stream.str().c_str());
		stream.str("");

		//Away
		stream << "Away: " << away_mean;
		printf("%-20s", stream.str().c_str());
		stream.str("");
		stream << "(" << away_lower << " - " << away_upper << ")";
		printf("%s\n", stream.str().c_str());
		stream.str("");
	

		//H2H
		stream << "H2H : " << mean_total;
		printf("%-20s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_lower + away_lower << " - " << home_upper + away_upper <<")";
		printf("%s\n\n", stream.str().c_str());
		stream.str("");

		stream << "Exponential smoothing\n";
		stream << "Home : " << home_exp_pred << "\nAway : " << away_exp_pred << "\nTotal: " << home_exp_pred + away_exp_pred << "\n\n";

		stream << "Linear regression\n";
		stream << "Home : " << home_regression_pred << "\nAway : " << away_regression_pred << "\nTotal: " << home_regression_pred + away_regression_pred << "\n\n";
		

		/*stream << "ARIMA\n";
		stream << "Home : " << arima_home_pred << "\nAway : " << arima_away_pred << "\nTotal: " << arima_home_pred + arima_away_pred << "\n\n\n";*/


		printf("%s", stream.str().c_str());
		get_recommendation(home_mean + away_mean, home_exp_pred + away_exp_pred, home_regression_pred + away_regression_pred, home_lower + away_lower, home_upper + away_upper);

		stream.str("");
		stream.clear();

		match_string.clear();
		match_details.clear();
#ifdef _MSC_VER
		SetConsoleTextAttribute(hConsole, 7);
#endif // _MSC_VER
	}//while

	file.close();
	return 0;
}