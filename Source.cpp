#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <string>
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

std::vector<std::string> split_string(const char* str, size_t length) {
	std::vector<std::string> split_str;
	std::string s{};
	int i = 0;
	while (!std::isdigit(str[i])) ++i;

	//insert team name
	split_str.push_back(trim(std::string(str, i)));

	for (; i < length; ++i) {
		if (!isspace(str[i]))
			s.push_back(str[i]);
		else {
			if (s.size() > 0) {
				split_str.push_back(s);
				s.clear();
			}
		}
	}
	return split_str;
}


float fulltime_mean(const std::vector<int>& scores) {
	float result{ 0. };
	int size = scores.size();
	for (unsigned i{ 0 }; i < size; ++i) {
		result += scores[i];
	}
	result /= float(size);
	return result;
}

float standard_deviation(const std::vector<int>& scores, float mean) {
	float variance{ 0 };
	int size = scores.size();
	for (unsigned i{ 0 }; i < size; ++i) {
		variance += std::pow((mean - scores[i]), 2);
	}
	variance /= ((float)size - 1);
	return std::sqrt(variance);
}


int main(int argc, char* argv[]) {
	//only basketball is currently supported.
	//Future sports include, Tennis, Baseball, Hockey and Football.
	if (argc < 2) {
		printf("Usage: Rehoboam [filename]\n");
		exit(1);
	}
	const char* file_path = argv[1];
	std::vector<int> home_score, away_score;
	std::string match_details;
	std::string home_team{}, away_team{};
	std::string match;
	std::string line;
	std::vector<int> scores;
	std::fstream file{ file_path, std::ios::in | std::ios::binary};
	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}
	std::ostringstream stream;

	//This loop reads all non-empty lines in the file. Three lines in a single loop.
	while (!file.eof()) {
		//Read until a non-empty line is found
		while (std::getline(file, line) && line.size() <= 1) continue;
		if (file.eof()) break;
		std::vector<std::string> str_token = split_string(line.c_str(), line.size());
		match += str_token[0];
		for (int i = 1; i < str_token.size(); ++i) {
			home_score.push_back((int)(std::stoi(str_token[i])));
		}
		line.clear();

		while (std::getline(file, line) && line.size() <= 1) continue;
		str_token = split_string(line.c_str(), line.size());
		match += " vs " + str_token[0];
		for (int i = 1; i < str_token.size(); ++i) {
			away_score.push_back((int)(std::stoi(str_token[i])));
		}
		line.clear();

		while (std::getline(file, match_details) && match_details.size() <= 1) continue;

		//..........................................................................................................................
		float home_mean{ 0.0 }, away_mean{ 0.0 };
		float home_stddev{ 0.0 }, away_stddev{ 0.0 };

		//Normal distribution calculation
		home_mean = fulltime_mean(home_score);
		home_stddev = standard_deviation(home_score, home_mean);
		away_mean = fulltime_mean(away_score);
		away_stddev = standard_deviation(away_score, away_mean);
		float mean_total = home_mean + away_mean;

		float total_mean_lower_bound = (home_mean - home_stddev) + (away_mean - away_stddev);
		float total_mean_upper_bound = (home_mean + home_stddev) + (away_mean + away_stddev);

		//T-distribution calculation
		float t_upper_bound{ 0.0 }, t_lower_bound{ 0.0 };

		t_dist(home_score, home_mean, home_stddev, &t_lower_bound, &t_upper_bound);
		float home_tdist_lowwer{ t_lower_bound }, home_tdist_upper{ t_upper_bound };

		t_dist(away_score, away_mean, away_stddev, &t_lower_bound, &t_upper_bound);
		float away_tdist_lower{ t_lower_bound }, away_tdist_upper{ t_upper_bound };

		//Exponential Smoothing
		float home_exp_pred = exponential_smoothing(home_score);
		float away_exp_pred = exponential_smoothing(away_score);

		//Simple Linear Regression
		float home_regression_pred = simple_linear_regression(home_score);
		float away_regression_pred = simple_linear_regression(away_score);

		//ARIMA
		/*double arima_home_pred = predictARIMA(home_score);
		double arima_away_pred = predictARIMA(away_score);*/

		//.......................................................................................................................

#ifdef _MSC_VER
		std::random_device rd{};
		auto mtgen = std::mt19937{ rd() };
		auto ud = std::uniform_int_distribution<int>{ 1, 5 };
		int random_number = ud(mtgen);
		int text_color = 0;
		switch (random_number) {
		case 1: text_color = 1;
			break;
		case 2: text_color = 2;
			break;
		case 3: text_color = 4;
			break;
		case 4: text_color = 6;
			break;
		case 5: text_color = 13;
			break;
		}
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, text_color | FOREGROUND_INTENSITY);
#endif // Change console color

		//Display results
		std::string design(match_details.size(), '.');
		std::cout << design<<"\n";
		std::cout << match_details << "\n";
		std::cout << design << "\n";

		const char* str1 = "Z-Distribution";
		const char* str2 = "T-Distribution";
		printf("%s\n",match.c_str());
		std::cout << std::setw(30) << std::right << str1 << std::setw(25) << std::right << str2 << "\n";
		
		//Home
		stream << "Home: " << home_mean;
		printf("%-15s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_mean - home_stddev << " - " << home_mean + home_stddev << ")";
		printf("%-25s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_tdist_lowwer<<" - " << home_tdist_upper<<")\n";
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
		stream << "(" << home_tdist_lowwer + away_tdist_lower << " - " << home_tdist_upper + away_tdist_upper << ")\n\n";

		stream << "Exponential smoothing\n";
		stream << "Home : " << home_exp_pred << "\nAway : " << away_exp_pred << "\nTotal: " << home_exp_pred + away_exp_pred << "\n\n";
		stream << "Linear regression\n";
		stream << "Home : " << home_regression_pred << "\nAway : " << away_regression_pred << "\nTotal: " << home_regression_pred + away_regression_pred << "\n\n";

		/*stream << "ARIMA\n";
		stream << "Home : " << arima_home_pred << "\nAway : " << arima_away_pred << "\nTotal: " << arima_home_pred + arima_away_pred << "\n\n\n";*/


		printf("%s", stream.str().c_str());
		stream.str("");
		stream.clear();

		match.clear();
		home_score.clear();
		away_score.clear();
		match_details.clear();
#ifdef _MSC_VER
		SetConsoleTextAttribute(hConsole, 7);
#endif // _MSC_VER
	}//while

	file.close();
	return 0;
}