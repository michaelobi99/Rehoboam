#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <string>
#include <tuple>
#include <ranges>
#include "T-distribution.h"
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


float fulltime_mean(const std::vector<int>& scores) {
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
	float variance{ 0 };
	int size = scores.size();
	for (unsigned i{ 0 }; i < size; ++i) {
		variance += std::pow((mean - scores[i]), 2);
	}
	variance /= ((float)size - 1);
	return std::sqrt(variance);
}

void get_recommendation(float pred_1, float pred_2, float pred_3) {
	float avg_points = 0.4f * pred_1 + 0.4f * pred_2 + 0.2f * pred_3;
	const int max = 7;

	if (std::abs(pred_1 - pred_2) < max && std::abs(pred_1 - pred_3) < max && std::abs(pred_2 - pred_3) < max) {
		printf("RECOMMENDATION: High confidence - Expect about %.2f total points.\n\n", avg_points);
	}
	else {
		printf("RECOMMENDATION: Low confidence - Expect about %.2f total points.\n\n", avg_points);
	}
}


int main(int argc, char* argv[]) {
	//only basketball is currently supported.
	//Future sports include, Tennis, Baseball, Hockey and Football.
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

		float home_h2h_mean = fulltime_mean(home_h2h_scores);
		float away_h2h_mean = fulltime_mean(away_h2h_scores);

		//Exponential Smoothing
		float home_exp_pred = exponential_smoothing(home_past_scores) * ((home_h2h_mean == 0.f) ? 1.0 : 0.8) + home_h2h_mean * 0.2;
		//float home_adaptive_exp_pred = adaptive_exponential_smoothing(home_score);
		float away_exp_pred = exponential_smoothing(away_past_scores) * ((away_h2h_mean == 0.f) ? 1.0 : 0.8) + away_h2h_mean * 0.2;
		//float away_adaptive_exp_pred = adaptive_exponential_smoothing(away_score);

		//Simple Linear Regression
		float home_regression_pred = simple_linear_regression(home_past_scores) * ((home_h2h_mean == 0.f) ? 1.0 : 0.8) + home_h2h_mean * 0.2;
		float away_regression_pred = simple_linear_regression(away_past_scores) * ((away_h2h_mean == 0.f) ? 1.0 : 0.8) + away_h2h_mean * 0.2;


		//Normal distribution calculation
		float home_mean{ 0.0 }, away_mean{ 0.0 };
		float home_stddev{ 0.0 }, away_stddev{ 0.0 };

		std::copy(home_h2h_scores.begin(), home_h2h_scores.end(), std::back_inserter(home_past_scores));
		std::copy(away_h2h_scores.begin(), away_h2h_scores.end(), std::back_inserter(away_past_scores));

		home_mean = fulltime_mean(home_past_scores);
		home_stddev = standard_deviation(home_past_scores, home_mean);
		away_mean = fulltime_mean(away_past_scores);
		away_stddev = standard_deviation(away_past_scores, away_mean);

		float mean_total = home_mean + away_mean;
		float total_mean_lower_bound = (home_mean - home_stddev) + (away_mean - away_stddev);
		float total_mean_upper_bound = (home_mean + home_stddev) + (away_mean + away_stddev);

		//T-distribution calculation
		float t_upper_bound{ 0.0 }, t_lower_bound{ 0.0 };

		t_dist(home_past_scores, home_mean, home_stddev, &t_lower_bound, &t_upper_bound);
		float home_tdist_lower{ t_lower_bound }, home_tdist_upper{ t_upper_bound };

		t_dist(away_past_scores, away_mean, away_stddev, &t_lower_bound, &t_upper_bound);
		float away_tdist_lower{ t_lower_bound }, away_tdist_upper{ t_upper_bound };

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

		const char* str1 = "Z-Distribution";
		const char* str2 = "T-Distribution";
		std::cout << std::setw(30) << std::right << str1 << std::setw(25) << std::right << str2 << "\n";
		
		//Home
		stream << "Home: " << home_mean;
		printf("%-15s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_mean - home_stddev << " - " << home_mean + home_stddev << ")";
		printf("%-25s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_tdist_lower<<" - " << home_tdist_upper<<")\n";
		printf("%s", stream.str().c_str());
		stream.str("");

		//Away
		stream << "Away: " << away_mean;
		printf("%-15s", stream.str().c_str());
		stream.str("");
		stream << "(" << away_mean - away_stddev << " - " << away_mean + away_stddev << ")";
		printf("%-25s", stream.str().c_str());
		stream.str("");
		stream << "(" << away_tdist_lower << " - " << away_tdist_upper << ")\n";
		printf("%s", stream.str().c_str());
		stream.str("");

		//H2H
		stream << "H2H : " << mean_total;
		printf("%-15s", stream.str().c_str());
		stream.str("");
		stream << "(" << total_mean_lower_bound << " - " << total_mean_upper_bound<<")";
		printf("%-25s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_tdist_lower + away_tdist_lower << " - " << home_tdist_upper + away_tdist_upper << ")\n\n";

		stream << "Exponential smoothing\n";
		stream << "Home : " << home_exp_pred
			<< "\nAway : " << away_exp_pred
			<< "\nTotal: " << home_exp_pred + away_exp_pred << "\n\n";

		stream << "Linear regression\n";
		stream << "Home : " << home_regression_pred << "\nAway : " << away_regression_pred << "\nTotal: " << home_regression_pred + away_regression_pred << "\n\n";
		

		/*stream << "ARIMA\n";
		stream << "Home : " << arima_home_pred << "\nAway : " << arima_away_pred << "\nTotal: " << arima_home_pred + arima_away_pred << "\n\n\n";*/


		printf("%s", stream.str().c_str());
		get_recommendation(home_mean + away_mean, home_exp_pred + away_exp_pred, home_regression_pred + away_regression_pred);

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