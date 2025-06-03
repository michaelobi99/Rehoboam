#pragma once
#include <string>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <map>

#ifdef _MSC_VER
#include <Windows.h>
#endif


class Player {
public:
	Player() = default;

	Player(std::string name, unsigned ranking, unsigned elo_ranking, float elo_score, unsigned clay_elo, float clay_elo_score, unsigned hard_elo, float hard_elo_score,
		unsigned grass_elo, float grass_elo_score, unsigned age, char gender) :
		name{ std::move(name) },
		ranking{ ranking },
		elo_ranking{ elo_ranking },
		elo_score{ elo_score },
		clay_elo_ranking{ clay_elo },
		clay_elo_score{ clay_elo_score },
		hard_elo_ranking{ hard_elo },
		hard_elo_score{ hard_elo_score },
		grass_elo_ranking{ grass_elo },
		grass_elo_score{ grass_elo_score },
		age{ age },
		gender{gender}
	{}

	friend void compare_profile(Player&, Player&, std::string, std::ostringstream&);

private:
	
	std::string name;
	unsigned ranking;
	unsigned elo_ranking;
	float elo_score;
	unsigned clay_elo_ranking;
	float clay_elo_score;
	unsigned hard_elo_ranking;
	float hard_elo_score;
	unsigned grass_elo_ranking;
	float grass_elo_score;
	unsigned age;
	char gender;
};

void compare_profile(Player& player1, Player& player2, std::string surface, std::ostringstream& stream) {
	std::string tournament_str = player1.gender == 'M' ? "ATP" : "WTA";
	std::transform(surface.begin(), surface.end(), surface.begin(), [](unsigned char c) { return std::tolower(c); });
	stream << std::left << std::setw(17) << "Name:" << std::left << std::setw(25) << player1.name << player2.name << "\n";
	stream << std::left << std::setw(17) << "Age:" << std::left << std::setw(25) << player1.age << player2.age << "\n";
	stream << std::left << std::setw(17) << tournament_str + " Ranking:" << std::left << std::setw(25) << player1.ranking << player2.ranking << "\n";
	float player1_elo_score{ 0.0 }, player2_elo_score{ 0.0 };
	if (surface.contains("hard")) {
		stream << std::left << std::setw(17) << "ELO_Hard" << std::left << std::setw(25) << player1.hard_elo_ranking << player2.hard_elo_ranking << "\n";
		player1_elo_score = player1.hard_elo_score;
		player2_elo_score = player2.hard_elo_score;
	}
	else if (surface.contains("clay")) {
		stream << std::left << std::setw(17) << "ELO_Clay:" << std::left << std::setw(25) << player1.clay_elo_ranking << player2.clay_elo_ranking << "\n";
		player1_elo_score = player1.clay_elo_score;
		player2_elo_score = player2.clay_elo_score;
	}
	else {
		stream << std::left << std::setw(17) << "ELO_Grass:" << std::left << std::setw(25) << player1.grass_elo_ranking << player2.grass_elo_ranking << "\n";
		player1_elo_score = player1.grass_elo_score;
		player2_elo_score = player2.grass_elo_score;
	}
	std::ostringstream prob_stream;
	prob_stream << std::setprecision(4);
	float match_win_prob_for_p1 = (1 / (float)(1 + std::pow(10, ((player2_elo_score - player1_elo_score) / float(400)))))  * 100;
	prob_stream << match_win_prob_for_p1 << "%";
	std::string p1_win_prob_str = prob_stream.str();
	prob_stream.str("");


	float match_win_prob_for_p2{ 100 - match_win_prob_for_p1 };
	prob_stream << match_win_prob_for_p2 << "%";
	std::string p2_win_prob_str = prob_stream.str();
	prob_stream.str("");

	if (player1_elo_score != 0 && player2_elo_score != 0) {
		stream << std::left << std::setw(17) << "Win Probability:" << std::left << std::setw(25) << p1_win_prob_str << p2_win_prob_str << "\n";
	}
	else
		stream << std::left << std::setw(10) << "Win Probability:" << std::left << std::setw(25) << " " << " " << "\n\n";
	
}


std::string trim_(const std::string& str) {
	auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
		return std::isspace(ch);
	});

	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
		return std::isspace(ch);
	}).base();

	return (start < end) ? std::string(start, end) : "";
}

std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(trim_(token));
	}
	return tokens;
}

std::string replace_char(const std::string& str, char ch) {
	std::string result = str;
	std::replace(result.begin(), result.end(), ch, ' ');
	return result;
}

std::tuple<std::string, std::string> split_players_names(const std::string& line) {
	std::string separator = " vs ";
	size_t pos = line.find(separator);
	if (pos != std::string::npos) {
		std::string player1 = line.substr(0, pos);
		std::string player2 = line.substr(pos + separator.length());

		// Apply transformations
		player1 = trim_(replace_char(player1, '-'));
		player2 = trim_(replace_char(player2, '-'));

		if (!player1.empty() && !player2.empty()) {
			return std::make_tuple(player1, player2);
		}
	}
	return std::make_tuple(std::string(), std::string());
}


bool same_name(std::string str1, std::string str2) {
	std::vector<std::string> str1_tokens{ split(str1, ' ') };
	std::vector<std::string> str2_tokens{ split(str2, ' ') };

	for (auto& str : str1_tokens) str = trim_(replace_char(str, '.'));
	for (auto& str : str2_tokens) str = trim_(replace_char(str, '.'));


	std::string longest_name{};
	auto iter = std::max_element(str1_tokens.begin(), str1_tokens.end(),
		[](std::string& a, std::string& b) {return a.size() < b.size(); });

	if (iter != str1_tokens.end()) longest_name = *iter;

	bool found = false;

	for (auto name : str2_tokens) {
		if (longest_name == name) {
			found = true;
			break;
		}
	}
	if (!found) return false;

	unsigned counter = 0;

	for (auto n1 : str1_tokens) {
		for (auto n2 : str2_tokens) {
			if (n2.starts_with(n1)) {
				++counter;
				break;
			}
		}
	}
	return counter > 1;
}

std::tuple<Player, Player> make_players_profile(std::string player1, std::string player2, std::fstream& file, char gender) {
	std::string line;
	std::vector<Player> players(2);

	// Reset file to beginning
	file.clear();
	file.seekg(0, std::ios::beg);
	int counter = 0;


	// Skip header line
	if (std::getline(file, line)) {
		// Process each data line
		while (std::getline(file, line)) {
			if (line.empty()) continue;
			std::vector<std::string> fields = split(line, ',');

			// Ensure we have enough fields
			if (fields.size() < 14) continue;


			// Extract and clean the player name (index 1)
			std::string player_name = trim_(replace_char(fields[1], '-'));


			// Check if this is one of our target players
			if (same_name(player1, player_name)) {
				try {
					unsigned elo_rank = static_cast<unsigned>(std::stoi(trim(fields[0])));     // Elo Rank
					double age_double = std::stod(trim(fields[2]));                            // Age
					unsigned age = static_cast<unsigned>(age_double);
					unsigned atp_rank = static_cast<unsigned>(std::stoi(trim(fields[12])));   // ATP Rank
					unsigned hard_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[4]))); // hElo Rank
					unsigned clay_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[6]))); // cElo Rank  
					unsigned grass_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[8]))); // gElo Rank
					float elo_score = static_cast<float>(std::stod(trim(fields[3])));
					float hard_elo_score = static_cast<float>(std::stod(trim(fields[5])));
					float clay_elo_score = static_cast<float>(std::stod(trim(fields[7])));
					float grass_elo_score = static_cast<float>(std::stod(trim(fields[9])));

					players[0] = Player(player_name, atp_rank, elo_rank, elo_score,
						clay_elo_rank, clay_elo_score, hard_elo_rank, hard_elo_score, grass_elo_rank, grass_elo_score, age, gender);
					++counter;
				}
				catch (const std::exception& e) {
					continue;
				}
			}else if (same_name(player2, player_name)) {
				try {
					unsigned elo_rank = static_cast<unsigned>(std::stoi(trim(fields[0])));     // Elo Rank
					double age_double = std::stod(trim(fields[2]));                            // Age
					unsigned age = static_cast<unsigned>(age_double);
					unsigned atp_rank = static_cast<unsigned>(std::stoi(trim(fields[12])));   // ATP Rank
					unsigned hard_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[4]))); // hElo Rank
					unsigned clay_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[6]))); // cElo Rank  
					unsigned grass_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[8]))); // gElo Rank
					float elo_score = static_cast<float>(std::stod(trim(fields[3])));
					float hard_elo_score = static_cast<float>(std::stod(trim(fields[5])));
					float clay_elo_score = static_cast<float>(std::stod(trim(fields[7])));
					float grass_elo_score = static_cast<float>(std::stod(trim(fields[9])));

					players[1] = Player(player_name, atp_rank, elo_rank, elo_score,
						clay_elo_rank, clay_elo_score, hard_elo_rank, hard_elo_score, grass_elo_rank, grass_elo_score, age, gender);
					++counter;
				}
				catch (const std::exception& e) {
					continue;
				}
			}
			if (counter == 2) {
				break;
			}
		}
	}
	
	return std::make_tuple(players[0], players[1]);
}

void process_tennis_file(std::string const& game_file_path) {
	std::string players_string{ "" }, surface{ "" }, tournament{ "" };
	std::string design(80, '-');
	std::fstream file{ game_file_path, std::ios::in | std::ios::binary };

	std::string male_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\men_elo_rankings.csv)";
	std::string female_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\women_elo_rankings.csv)";
	std::fstream profile_file;

	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}
	std::ostringstream stream;

	char gender = 'M';
	if (game_file_path.contains("ATP"))
		profile_file.open(male_file, std::ios::in);
	else {
		gender = 'F';
		profile_file.open(female_file, std::ios::in);
	}


	//This loop reads all non-empty lines in the file. Three lines in a single loop.
	while (!file.eof()) {
		//Read until a non-empty line is found
		while (std::getline(file, players_string) && players_string.size() <= 1) continue;
		if (file.eof()) break;
		while (std::getline(file, surface) && surface.size() <= 1) continue;
		trim_(surface);
		while (std::getline(file, tournament) && tournament.size() <= 1) continue;
		trim_(tournament);

		auto [player1, player2] = split_players_names(players_string.c_str());

		auto [player_1, player_2] = make_players_profile(player1, player2, profile_file, gender);
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
		stream << design << "\n";
		compare_profile(player_1, player_2, surface, stream);
		stream << design << "\n\n";

		std::cout << stream.str();

		stream.str("");
		stream.clear();
		surface.clear();
		players_string.clear();
		tournament.clear();
#ifdef _MSC_VER
		SetConsoleTextAttribute(hConsole, 7);
#endif // _MSC_VER
	}//while

	file.close();
	profile_file.close();
}