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

	Player(std::string name, unsigned ranking, unsigned elo_ranking, unsigned clay_elo,
		unsigned hard_elo, unsigned grass_elo, unsigned age) :
		name{ std::move(name) },
		ranking{ ranking },
		elo_ranking{ elo_ranking },
		clay_elo_ranking{ clay_elo },
		hard_elo_ranking{ hard_elo },
		grass_elo_ranking{ grass_elo },
		age{ age }
	{}

	void compare_profile(Player const& other, std::string surface, std::ostringstream& stream) const {
		std::transform(surface.begin(), surface.end(), surface.begin(), [](unsigned char c) { return std::tolower(c); });
		stream << std::left << std::setw(10) << "Name:" << std::left << std::setw(30) << name << other.name << "\n";
		stream << std::left << std::setw(10) << "Age:" << std::left << std::setw(30) << age << other.age << "\n";
		stream << std::left << std::setw(10) << "Ranking:" << std::left << std::setw(30) << ranking << other.ranking << "\n";
		if (surface.contains("hard"))
			stream << std::left << std::setw(10) << "ELO_Hard" << std::left << std::setw(30) << hard_elo_ranking << other.hard_elo_ranking << "\n";
		else if (surface.contains("clay"))
			stream << std::left << std::setw(10) << "ELO_Clay:" << std::left << std::setw(30) << clay_elo_ranking << other.clay_elo_ranking << "\n";
		else
			stream << std::left << std::setw(10) << "ELO_Grass:" << std::left << std::setw(30) << grass_elo_ranking << other.grass_elo_ranking << "\n\n";
	}

private:
	std::string name;
	unsigned ranking;
	unsigned elo_ranking;
	unsigned clay_elo_ranking;
	unsigned hard_elo_ranking;
	unsigned grass_elo_ranking;
	unsigned age;
};


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
	//std::replace(result.begin(), result.end(), '.', ' ');
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

	if (std::size(str1_tokens) != std::size(str2_tokens)) {
		return false;
	}
	unsigned counter = 0;

	for (auto n1 : str1_tokens) {
		for (auto n2 : str2_tokens) {
			if (n2.starts_with(n1)) {
				++counter;
				break;
			}
		}
	}
	return counter == str1_tokens.size();
}

std::tuple<Player, Player> make_players_profile(std::string player1, std::string player2, std::fstream& file) {
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
					// Parse the required fields
					unsigned elo_rank = static_cast<unsigned>(std::stoi(trim(fields[0])));     // Elo Rank
					double age_double = std::stod(trim(fields[2]));                            // Age
					unsigned age = static_cast<unsigned>(age_double);
					unsigned atp_rank = static_cast<unsigned>(std::stoi(trim(fields[12])));   // ATP Rank
					unsigned hard_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[4]))); // hElo Rank
					unsigned clay_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[6]))); // cElo Rank  
					unsigned grass_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[8]))); // gElo Rank

					players[0] = Player(player_name, atp_rank, elo_rank,
						clay_elo_rank, hard_elo_rank, grass_elo_rank, age);
					++counter;
				}
				catch (const std::exception& e) {
					// Skip malformed lines
					continue;
				}
			}else if (same_name(player2, player_name)) {
				try {
					// Parse the required fields
					unsigned elo_rank = static_cast<unsigned>(std::stoi(trim(fields[0])));     // Elo Rank
					double age_double = std::stod(trim(fields[2]));                            // Age
					unsigned age = static_cast<unsigned>(age_double);
					unsigned atp_rank = static_cast<unsigned>(std::stoi(trim(fields[12])));   // ATP Rank
					unsigned hard_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[4]))); // hElo Rank
					unsigned clay_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[6]))); // cElo Rank  
					unsigned grass_elo_rank = static_cast<unsigned>(std::stoi(trim(fields[8]))); // gElo Rank

					players[1] = Player(player_name, atp_rank, elo_rank,
						clay_elo_rank, hard_elo_rank, grass_elo_rank, age);
					++counter;
				}
				catch (const std::exception& e) {
					// Skip malformed lines
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

	if (game_file_path.contains("ATP"))
		profile_file.open(male_file, std::ios::in);
	else
		profile_file.open(female_file, std::ios::in);


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

		auto [player_1, player_2] = make_players_profile(player1, player2, profile_file);
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
		player_1.compare_profile(player_2, surface, stream);
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