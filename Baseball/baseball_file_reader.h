#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <numeric>
#include <ranges>

#ifdef _MSC_VER
#include <Windows.h>
#endif

using constRefVec = std::vector<int> const&;


float predict_total(float home_mean, float away_mean, constRefVec home_h2h_scores, constRefVec away_h2h_scores) {
	float home_avg = home_mean;
	float away_avg = away_mean;
	float h2h_avg = mean(home_h2h_scores) + mean(away_h2h_scores);
	float factor = h2h_avg == 0 ? 1.0 : 0.5;
	return int((home_avg + away_avg) * factor + h2h_avg * 0.5);
}

void read_and_process_file(std::string const& file_path) {
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

		line.clear();
		while (std::getline(file, match_details) && match_details.size() <= 1) continue;

		
		float home_mean = mean(home_past_scores);
		float home_stddev = standard_deviation(home_past_scores, home_mean);
		float away_mean = mean(away_past_scores);
		float away_stddev = standard_deviation(away_past_scores, away_mean);
		float mean_total = home_mean + away_mean;

		float home_lower{ 0 }, away_lower{ 0 };
		float home_upper{ 0 }, away_upper{ 0 };

		std::tie(home_lower, home_upper) = t_dist(home_past_scores.size(), home_mean, home_stddev);
		std::tie(away_lower, away_upper) = t_dist(away_past_scores.size(), away_mean, away_stddev);

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

		const char* str1 = "Confidence Interval";
		const char* str2 = "Recent game points";
		std::cout << std::setw(25) << std::right << str1 << "\n";

		//Home
		stream << "Home: ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "[" << home_lower << " - " << home_upper << "]";
		printf("%s\n", stream.str().c_str());
		stream.str("");

		//Away
		stream << "Away: ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "[" << away_lower << " - " << away_upper << "]";
		printf("%s\n", stream.str().c_str());
		stream.str("");

		//H2H
		stream << "H2H : ";
		printf("%-5s", stream.str().c_str());
		stream.str("");
		stream << "[" << home_lower + away_lower << " - " << home_upper + away_upper << "]";
		printf("%s\n\n", stream.str().c_str());
		stream.str("");

		stream << "Total Predicted Runs: " << predict_total(home_mean, away_mean, home_h2h_scores, away_h2h_scores) << "\n\n";

		auto print_vec = []<typename T>(std::vector<T>&vec, std::ostringstream & stream) {
			for (T elem : vec) { stream << std::left << std::setw(4) << elem; }
			stream << "\n";
		};

		stream << std::setw(25) << std::left << str2 << "\n";
		stream << "Home: ";  print_vec(home_past_scores, stream);
		stream << "Away: ";  print_vec(away_past_scores, stream);
		stream << "\n";

		printf("%s", stream.str().c_str());

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

void process_baseball_file(std::string const& file_path) {
	read_and_process_file(file_path);
}