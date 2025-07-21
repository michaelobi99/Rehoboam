#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <ranges>
#include "bb_predictors.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

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

	std::string h2h_scores_str{ "" };
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


std::tuple<std::string, std::vector<int>> split_string_2(const char* str, size_t length) {
	std::istringstream stream(str);
	std::string name;
	std::getline(stream, name, ':');
	name = trim(name);

	std::string past_scores_str{ "" };
	std::getline(stream, past_scores_str, ':');
	past_scores_str = trim(past_scores_str);

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
	return std::tuple(name, past_scores);
}

void get_h2h_score_count(std::string const& line, unsigned& count) {
	unsigned pointer = line.size() - 1;
	std::string score_count{ "00" };
	while (!std::isdigit(line[pointer]) && pointer > 0) --pointer;
	score_count[1] = line[pointer];
	--pointer;
	if (std::isdigit(line[pointer]) && pointer > 0) {
		score_count[0] = line[pointer];
		--pointer;
	}
	count = std::stoi(score_count);
	return;
}

void split_quaters_score(std::string line, unsigned length, int& full_time_score, int& q1_score, int& q2_score, int& q3_score, int& q4_score) {
	unsigned index = 0;
	while (!std::isdigit(line[index])) index++;

	std::vector<int> row;

	std::string s{ "" };
	for (; index < line.size(); ++index) {
		if (isdigit(line[index]))
			s.push_back(line[index]);
		else {
			if (s.size() > 0) {
				row.push_back(std::stoi(s));
				s.clear();
			}
		}
	}

	full_time_score = row[0];
	q1_score += row[1];
	q2_score += row[2];
	q3_score += row[3];
	q4_score += row[4];
}


void get_recommendation(float pred, float low, float high) {
	const int max_dist_range = 24;
	bool good_pred = low <= pred && pred <= high;
	bool tight_range = (int)(high - low) <= max_dist_range;
	if (good_pred && tight_range) {
		printf("CONFIDENCE: HIGH\n\n");
	}
	else {
		printf("CONFIDENCE: LOW\n\n");
	}
}

float combine_predictions(float pred_1, float pred_2, float pred_3) {
	return (1/3.f) * (pred_1 + pred_2 + pred_3);
}

void read_and_process_quaters_file(std::string const& file_path) {
	std::string match_string{ "" }, match_details{ "" };
	unsigned h2h_scores_count{};
	std::fstream file{ file_path, std::ios::in | std::ios::binary };
	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}
	std::string line{};
	std::ostringstream stream;


	while (!file.eof()) {
		//Read until a non-empty line is found
		line.clear();
		while (std::getline(file, line) && line.size() <= 1) continue;
		if (file.eof()) break;

		auto [home_team_name, home_past_scores] = split_string_2(line.c_str(), line.size());
		match_string += home_team_name;

		line.clear();
		while (std::getline(file, line) && line.size() <= 1) continue;

		auto [away_team_name, away_past_scores] = split_string_2(line.c_str(), line.size());
		match_string += " vs " + away_team_name;

		line.clear();
		while (std::getline(file, line) && line.size() <= 1) continue;

		get_h2h_score_count(line, h2h_scores_count);
		std::vector<int> home_h2h_scores(h2h_scores_count, 0);
		std::vector<int> away_h2h_scores(h2h_scores_count, 0);
		std::vector<std::vector<int>> quaters_score(h2h_scores_count, std::vector<int>(4, 0));

		for (unsigned i{ 0 }; i < h2h_scores_count; ++i) {
			line.clear();
			while (std::getline(file, line) && line.size() <= 1) continue;
			split_quaters_score(line, line.length(), home_h2h_scores[i], quaters_score[i][0], quaters_score[i][1], quaters_score[i][2], quaters_score[i][3]);
			line.clear();
			while (std::getline(file, line) && line.size() <= 1) continue;
			split_quaters_score(line, line.length(), away_h2h_scores[i], quaters_score[i][0], quaters_score[i][1], quaters_score[i][2], quaters_score[i][3]);
		}
		match_details.clear();
		while (std::getline(file, match_details) && match_details.size() <= 1) continue;

		//..........................................................................................................................
		//Exponential Smoothing
		float home_exp_pred = exponential_smoothing(home_past_scores);
		float away_exp_pred = exponential_smoothing(away_past_scores);

		//Simple Linear Regression
		float home_regression_pred = simple_linear_regression(home_past_scores);
		float away_regression_pred = simple_linear_regression(away_past_scores);

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
		std::string design(80, '-');
		std::cout << design << "\n";
		std::cout << match_string << " - " << match_details << "\n";
		std::cout << design << "\n";

		const char* str1 = "Likely point range";
		const char* str2 = "Recent game points";
		std::cout << std::setw(25) << std::right << str1 << "\n";

		//Home
		stream << "Home: ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_lower << " - " << home_upper << ")";
		printf("%s\n", stream.str().c_str());
		stream.str("");

		//Away
		stream << "Away: ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "(" << away_lower << " - " << away_upper << ")";
		printf("%s\n", stream.str().c_str());
		stream.str("");

		//H2H
		stream << "H2H : ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "(" << home_lower + away_lower << " - " << home_upper + away_upper << ")";
		printf("%s\n\n", stream.str().c_str());
		stream.str("");


		auto print_vec = []<typename T>(std::vector<T>&vec, std::ostringstream & stream) {
			for (T elem : vec) { stream << std::left << std::setw(4) << elem; }
			stream << "\n";
		};

		std::cout << std::setw(25) << std::left << str2 << "\n";
		stream << "Home: ";  print_vec(home_past_scores, stream);
		stream << "Away: ";  print_vec(away_past_scores, stream);
		stream << "\n";


		stream << "GAME PREDICTION\n";
		float home_pred = combine_predictions(home_mean, home_exp_pred, home_regression_pred);
		float away_pred = combine_predictions(away_mean, away_exp_pred, away_regression_pred);
		float total = home_pred + away_pred;
		stream << "Home : " << home_pred << "\n";
		stream << "Away : " << away_pred << "\n";
		stream << "Total: " << total << "\n\n";

		printf("%s", stream.str().c_str());

		get_recommendation(total, home_lower + away_lower, home_upper + away_upper);

		//...................................................................................................................................................

		// Determine max size for iteration
		if (h2h_scores_count > 0) {
			printf("Recent H2H results\n");
			printf("%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s%-8s\n", "Home ", "Away ", "Total", "H1", "H2", "Q1", "Q2", "Q3", "Q4");
			std::cout << design << "\n";

		
			int total = 0;
			for (size_t i = 0; i < h2h_scores_count; ++i) {
				printf("%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d%-8d\n", home_h2h_scores[i], away_h2h_scores[i], home_h2h_scores[i] + away_h2h_scores[i], quaters_score[i][0] + quaters_score[i][1],
					quaters_score[i][2] + quaters_score[i][3], quaters_score[i][0], quaters_score[i][1], quaters_score[i][2], quaters_score[i][3]);
			}
		}

		//....................................................................................................................................................

		std::cout << design << "\n\n";
		stream.str("");
		stream.clear();

		match_string.clear();
		match_details.clear();
#ifdef _MSC_VER
		SetConsoleTextAttribute(hConsole, 7);
#endif // _MSC_VER
	}//while

	file.close();
}