#include "Elo.h"
#include <algorithm>

#ifdef _MSC_VER
#include <Windows.h>
#endif

struct PlayerATPRank {
	std::string rank;
	std::string name;
};

void print_player_stats(Player& player1, Player& player2, std::string surface, std::ostringstream& stream, std::string& design, std::string& tournament) {
	if (player1.name.empty() || player2.name.empty()) return;
	if (player1.age == 0 || player2.age == 0) return;
	if (player1.ranking == 0 || player2.ranking == 0) return;

	bool bestOf5 = false;
	std::vector<std::string> bestOf5Tournaments = { "French Open", "US Open", "Australian Open", "Wimbledon" };

	for (auto str : bestOf5Tournaments) {
		if (tournament.contains(str)) {
			bestOf5 = true;
			break;
		}
	}

	stream << design << "\n";
	stream << tournament << "\n";
	stream << design << "\n";
	std::transform(surface.begin(), surface.end(), surface.begin(), [](unsigned char c) { return std::tolower(c); });
	stream << std::left << std::setw(25) << "Name:" << std::left << std::setw(25) << player1.name << player2.name << "\n";
	stream << std::left << std::setw(25) << "Age:" << std::left << std::setw(25) << player1.age << player2.age << "\n";
	stream << std::left << std::setw(25) << "Official Ranking:" << std::left << std::setw(25) << player1.ranking << player2.ranking << "\n";
	stream << std::left << std::setw(25) << "Elo Ranking:" << std::left << std::setw(25) << player1.elo_ranking << player2.elo_ranking << "\n";
	float player1_elo_score{ 0.0 }, player2_elo_score{ 0.0 };
	if (surface.contains("hard")) {
		stream << std::left << std::setw(25) << "Hard Elo Rank:" << std::left << std::setw(25) << player1.hard_elo_ranking << player2.hard_elo_ranking << "\n";
		player1_elo_score = player1.hard_elo_score;
		player2_elo_score = player2.hard_elo_score;
	}
	else if (surface.contains("clay")) {
		stream << std::left << std::setw(25) << "Clay Elo Rank:" << std::left << std::setw(25) << player1.clay_elo_ranking << player2.clay_elo_ranking << "\n";
		player1_elo_score = player1.clay_elo_score;
		player2_elo_score = player2.clay_elo_score;
	}
	else {
		stream << std::left << std::setw(25) << "Grass Elo Rank:" << std::left << std::setw(25) << player1.grass_elo_ranking << player2.grass_elo_ranking << "\n";
		player1_elo_score = player1.grass_elo_score;
		player2_elo_score = player2.grass_elo_score;
	}

	// Calculate probabilities
	auto [player1_game_win_prob, player1_set_prob, player1_match_win_prob] =
		TennisEloPredictor::predictMatch(player1_elo_score, player2_elo_score, bestOf5);
	
	auto [player2_game_win_prob, player2_set_prob, player2_match_win_prob] =
		TennisEloPredictor::predictMatch(player2_elo_score, player1_elo_score, bestOf5);

	player1_game_win_prob *= 100;
	player1_set_prob *= 100;
	player1_match_win_prob *= 100;
	
	player2_game_win_prob *= 100;
	player2_set_prob *= 100;
	player2_match_win_prob *= 100;

	// Estimate match length
	auto [expectedGames, expectedSets] =
		TennisEloPredictor::estimateMatchLength(player1_elo_score, player2_elo_score, bestOf5);

	// Monte Carlo simulation
	auto [player1Wins, avgGames, exampleSetScores] =
		TennisEloPredictor::simulateMatch(player1_elo_score, player2_elo_score, bestOf5, 10000);

	player1Wins /= 100.0;
	double player2Wins = 100 - player1Wins;


	std::ostringstream prob_stream;

	prob_stream << player1_game_win_prob << "%";
	std::string player1_game_win_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player2_game_win_prob << "%";
	std::string player2_game_win_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player1_set_prob << "%";
	std::string player1_set_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player2_set_prob << "%";
	std::string player2_set_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player1_match_win_prob << "%";
	std::string player1_match_win_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player2_match_win_prob << "%";
	std::string player2_match_win_prob_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player1Wins << "%";
	std::string player1Wins_str = prob_stream.str();
	prob_stream.str("");

	prob_stream << player2Wins << "%";
	std::string player2Wins_str = prob_stream.str();
	prob_stream.str("");


	stream << std::left << std::setw(25) << "Game win probability:" << std::left << std::setw(25) << player1_game_win_prob_str << player2_game_win_prob_str << "\n";
	stream << std::left << std::setw(25) << "Set win probability:" << std::left << std::setw(25) << player1_set_prob_str << player2_set_prob_str << "\n";
	stream << std::left << std::setw(25) << "Match win Probability:" << std::left << std::setw(25) << player1_match_win_prob_str << player2_match_win_prob_str << "\n";
	stream << std::left << std::setw(25) << "simulated win rate:" << std::left << std::setw(25) << player1Wins_str << player2Wins_str << "\n";
	stream << "Simulated number of games in match: ";
	for (auto elem : avgGames)
		stream << elem << ". ";
	stream << "\n";
	stream << design << "\n\n";
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
	if (ch == '\'') {
		int i = 0;
		for (; i < std::size(str); ++i) {
			if (result[i] == '\'') break;
		}
		if (i < std::size(result)) {
			std::memmove(result.data() + i, result.data() + i + 1, std::size(result) - i);
			result.pop_back();
		}
	}
	else
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
	std::transform(std::begin(str1), std::end(str1), std::begin(str1), ::tolower);
	std::transform(std::begin(str2), std::end(str2), std::begin(str2), ::tolower);

	std::vector<std::string> str1_tokens{ split(str1, ' ') };
	std::vector<std::string> str2_tokens{ split(str2, ' ') };

	for (auto& str : str1_tokens) {
		str = trim_(replace_char(str, '.'));
		str = replace_char(str, '\'');
	}
	for (auto& str : str2_tokens) {
		str = trim_(replace_char(str, '.'));
		str = replace_char(str, '\'');
	}

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
	if (!found) {
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
	return counter > 1;
}

std::tuple<Player, Player> make_players_profile(std::string player1, std::string player2, std::fstream& file, const std::vector<PlayerATPRank>& atp_file, char gender) {
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
					unsigned atp_rank;
					for (auto player : atp_file) {
						if (same_name(player1, player.name)) {
							atp_rank = std::stoi(trim(player.rank));   // ATP Rank
							break;
						}
					}
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
					unsigned atp_rank;
					for (auto player : atp_file) {
						if (same_name(player2, player.name)) {
							atp_rank = std::stoi(trim(player.rank));   // ATP Rank
							break;
						}
							
					}
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
		//Players name not present in ELO top 400, try ATP top 1000.

	}
	
	return std::make_tuple(players[0], players[1]);
}

std::vector<PlayerATPRank> getRankings(const std::string& file) {
	std::fstream fileObj(file, std::ios::in);
	std::vector<PlayerATPRank> result;
	std::string line;
	std::getline(fileObj, line);
	while (std::getline(fileObj, line)) {
		std::string rank, name;
		size_t pos = 0;
		pos = line.find(',');
		if (pos != std::string::npos)
			rank = line.substr(0, pos);
		line.erase(0, pos + 1);
		pos = line.find(',');
		if (pos != std::string::npos) {
			name = trim_(replace_char(line.substr(0, pos), '-'));
		}
		
		result.emplace_back(PlayerATPRank{ rank, name });
	}
	return result;
}


void process_tennis_file(std::string const& game_file_path) {
	std::string players_string{ "" }, surface{ "" }, tournament{ "" };
	std::string design(80, '-');
	std::fstream file{ game_file_path, std::ios::in | std::ios::binary };

	std::string male_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\men_elo_rankings.csv)";
	std::string female_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\women_elo_rankings.csv)";
	std::string male_atp_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\atp_live_rankings.csv)";
	std::string women_atp_file = R"(C:\Users\HP\source\repos\Rehoboam\Rehoboam\Data\Tennis\wta_live_rankings.csv)";

	std::fstream profile_file;

	std::vector<PlayerATPRank> men_rank(getRankings(male_atp_file));
	std::vector<PlayerATPRank> women_rank(getRankings(women_atp_file));
	

	if (!file.is_open()) {
		std::cerr << "Failed to open file\n";
		exit(1);
	}
	std::ostringstream stream;

	char gender = 'M';
	if (game_file_path.contains("ATP") || game_file_path.contains("Challenger"))
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

		auto [player_1, player_2] = gender == 'M' ? make_players_profile(player1, player2, profile_file, men_rank, gender) :
			make_players_profile(player1, player2, profile_file, women_rank, gender);
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

		print_player_stats(player_1, player_2, surface, stream, design, tournament);

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