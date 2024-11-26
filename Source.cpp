#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <string>

#ifdef _MSC_VER
#include <Windows.h>
#endif // _MSC_VER


std::vector<std::string> split_string(const char* str, size_t length) {
	std::vector<std::string> split_str;
	std::string s{};
	for (int i = 0; i < length; ++i) {
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
	variance /= (float)size;
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
	std::vector<int> home_score, away_score, h2h_score;
	std::string home_team{}, away_team{};
	std::string match;
	std::string line;
	std::vector<int> scores;
	std::fstream file{ file_path, std::ios::in | std::ios::binary};
	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}

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

		while (std::getline(file, line) && line.size() <= 1) continue;
		str_token = split_string(line.c_str(), line.size());
		for (int i = 1; i < str_token.size(); ++i) {
			h2h_score.push_back((int)(std::stoi(str_token[i])));
		}
		line.clear();

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

		float home_average{ 0.0 }, away_average{ 0.0 }, h2h_average{ 0.0 };
		float home_std_deviation{ 0.0 }, away_std_deviation{ 0.0 }, h2h_std_deviation{ 0.0 };
		home_average = fulltime_mean(home_score); home_std_deviation = standard_deviation(home_score, home_average);
		away_average = fulltime_mean(away_score); away_std_deviation = standard_deviation(away_score, away_average);
		h2h_average = fulltime_mean(h2h_score); h2h_std_deviation = standard_deviation(h2h_score, h2h_average);

		float avg_total = home_average + away_average;
		float avg_lower_bound = (home_average - home_std_deviation) + (away_average - away_std_deviation);
		float avg_upper_bound = (home_average + home_std_deviation) + (away_average + away_std_deviation);

		printf("%s\n", match.c_str());
		printf("Home average: %.3f, Home bound: {%.3f - %.3f}\n", home_average, home_average - home_std_deviation, home_average + home_std_deviation);
		printf("Away average: %.3f, Away bound: {%.3f - %.3f}\n", away_average, away_average - away_std_deviation, away_average + away_std_deviation);
		//printf("H2H average:  %.3f, H2H  bound: {%.3f - %.3f}\n\n", h2h_average, h2h_average - h2h_std_deviation, h2h_average + h2h_std_deviation);
		printf("H2H average:  %.3f, H2H  bound: {%.3f - %.3f}\n\n", avg_total, avg_lower_bound, avg_upper_bound);
		match.clear();
		home_score.clear();
		away_score.clear();
		h2h_score.clear();
#ifdef _MSC_VER
		SetConsoleTextAttribute(hConsole, 7);
#endif // _MSC_VER
	}//while

	file.close();
	return 0;
}