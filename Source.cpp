#include <iostream>
#include <vector>
#include <Windows.h>

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
	std::vector<int> home_score, away_score, h2h_score;
	int score{ 0 };
	int count{ 0 };
	printf("How many home stats?: ");
	std::cin >> count;
	printf("Enter home stats:\n");
	for (int i = 0; i < count; i++) {
		std::cin >> score;
		home_score.push_back(score);
	}

	printf("How many away stats?: ");
	std::cin >> count;
	printf("Enter away stats:\n");
	for (int i = 0; i < count; i++) {
		std::cin >> score;
		away_score.push_back(score);
	}

	printf("How many h2h stats?: ");
	std::cin >> count;
	printf("Enter h2h stats:\n");
	for (int i = 0; i < count; i++) {
		std::cin >> score;
		h2h_score.push_back(score);
	}
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	float home_average{ 0.0 }, away_average{ 0.0 }, h2h_average{ 0.0 };
	float home_std_deviation{ 0.0 }, away_std_deviation{ 0.0 }, h2h_std_deviation{ 0.0 };
	home_average = fulltime_mean(home_score); home_std_deviation = standard_deviation(home_score, home_average);
	away_average = fulltime_mean(away_score); away_std_deviation = standard_deviation(away_score, away_average);
	h2h_average = fulltime_mean(h2h_score); h2h_std_deviation = standard_deviation(h2h_score, h2h_average);

	printf("Home average: %f, Home bound: {%f - %f}\n\n", home_average, home_average - home_std_deviation, home_average + home_std_deviation);
	printf("Away average: %f, Away bound: {%f - %f}\n\n", away_average, away_average - away_std_deviation, away_average + away_std_deviation);
	printf("H2H average: %f, H2H bound: {%f - %f}\n\n", h2h_average, h2h_average - h2h_std_deviation, h2h_average + h2h_std_deviation);
	SetConsoleTextAttribute(hConsole, 7);
	return 0;
}