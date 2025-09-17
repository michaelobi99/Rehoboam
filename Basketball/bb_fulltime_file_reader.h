#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <ranges>
#include "bb_quaters_file_reader.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif



void read_and_process_fulltime_file(std::string const& file_path) {
	std::string match_details;
	std::string match_string;
	std::string line;
	std::vector<int> scores;
	std::fstream file{ file_path, std::ios::in | std::ios::binary };
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

		/*std::cout << "Home score: ";
		std::for_each(std::begin(home_past_scores), std::end(home_past_scores), [](int s) {std::cout << s << "\n"; });
		std::for_each(std::begin(away_past_scores), std::end(away_past_scores), [](int s) {std::cout << s << "\n"; });*/

		line.clear();
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

		stream << "GAME PREDICTION\n";
		float home_pred = combine_predictions(home_mean, home_exp_pred, home_regression_pred);
		float away_pred = combine_predictions(away_mean, away_exp_pred, away_regression_pred);
		float total = home_pred + away_pred;
		stream << "Home : " << home_pred << "\n";
		stream << "Away : " << away_pred << "\n";
		stream << "Total: " << total << "\n\n";

		auto print_vec = []<typename T>(std::vector<T>&vec, std::ostringstream & stream) {
			for (T elem : vec) { stream << std::left << std::setw(4) << elem; }
			stream << "\n";
		};

		stream << std::setw(25) << std::left << str2 << "\n";
		stream << "Home: ";  print_vec(home_past_scores, stream);
		stream << "Away: ";  print_vec(away_past_scores, stream);
		stream << "\n";

		printf("%s", stream.str().c_str());

		//get_recommendation(total, home_lower + away_lower, home_upper + away_upper);

		// Determine max size for iteration
		if (home_h2h_scores.size() > 0) {
			printf("Recent H2H results\n");
			printf("%-15s %-15s %-15s\n", "Home", "Away", "Total");
			printf("%.*s\n", 45, "---------------------------------------------");

			if (home_h2h_scores.size() != away_h2h_scores.size()) {
				printf("ERROR: H2H score not consistent\n");
			}
			else {
				int total = 0;
				for (size_t i = 0; i < home_h2h_scores.size(); i++) {
					total = home_h2h_scores[i] + away_h2h_scores[i];
					printf("%-15d %-15d %-15d\n", home_h2h_scores[i], away_h2h_scores[i], total);
				}
			}
		}
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

void process_basketball_file(std::string const& file_path, bool quaters_file = false) {
	if (!quaters_file)
		read_and_process_fulltime_file(file_path);
	else
		read_and_process_quaters_file(file_path);
	
}