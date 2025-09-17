#pragma once

#include <iostream>
#include <cmath>
#include <vector>
#include <random>
#include <tuple>
#include <string>
#include <fstream>
#include <unordered_map>
#include <iomanip>


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
		gender{ gender }
	{}

	friend void print_player_stats(Player&, Player&, std::string, std::ostringstream&, std::string&, std::string&);

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


class TennisEloPredictor {
private:
	static constexpr double ELO_BASE = 10.0;
	static constexpr double ELO_SCALE = 400.0;
	static constexpr double BEST_OF_3_ALPHA = 7.0;
	static constexpr double BEST_OF_5_ALPHA = 11.5;
	
public:

	//These functions return probability for player1, player2_prob = 1-player1_prob
	static double calculateWinProbability(double player1_elo, double player2_elo) {
		double elo_diff = player1_elo - player2_elo;
		return 1.0 / (float)(1.0 + std::pow(ELO_BASE, -elo_diff / ELO_SCALE));
	}

	static double calculatePointWinProbability_1(double player1_elo, double player2_elo) {
		double matchProb = calculateWinProbability(player1_elo, player2_elo);
		double pointProb = 0.5 + (matchProb - 0.5) * 0.15;
		return max(0.0, min(1.0, pointProb));
	}

	static double calculatePointWinProbability_2(double player1_elo, double player2_elo, bool bestof5 = false) {
		double matchProb = calculateWinProbability(player1_elo, player2_elo);
		double alpha = bestof5 ? BEST_OF_5_ALPHA : BEST_OF_3_ALPHA;
		double alpha_inv = 1 / (float)alpha;
		double pointProb = std::pow(matchProb, alpha_inv) / (float)(std::pow(matchProb, alpha_inv) + std::pow(1 - matchProb, alpha_inv));
		return pointProb;
	}

	static double gameWinProbability(double pointProb) {
		if (pointProb == 0.5) return 0.5;

		double q = 1.0 - pointProb,
			   p = pointProb;
		//probability of winning 4-0, 4-1, 4-2
		double straightWin = std::pow(p, 4) + 4 * std::pow(p, 4) * q + 10 * std::pow(p, 4) * std::pow(q, 2);

		// Probability of winning from deuce (3-3) or (40-40)
		double deuceProb = 20 * std::pow(p, 3) * std::pow(q, 3) * (std::pow(p, 2) / (1 - 2 * p * q));

		return straightWin + deuceProb;
	}

	static double setWinProbability(double gameProb, double pointProb) {
		if (gameProb == 0.5) return 0.5;
		double result = 0.0;

		
		result += binomialCoeff(5, 0) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 0); //6-0
		result += binomialCoeff(6, 1) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 1); //6-1
		result += binomialCoeff(7, 2) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 2); //6-2
		result += binomialCoeff(8, 3) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 3); //6-3
		result += binomialCoeff(9, 4) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 4); //6-4

		// Win 6-0 to 6-4
		//for (int i = 0; i <= 4; ++i) {
		//	result += binomialCoeff(5 + i, i) * std::pow(gameProb, 6) * std::pow(1 - gameProb, i);
		//}


		// Win 7-5 (must play  5-5 first, then 6-5 and 7-5)
		result += binomialCoeff(10, 5) * std::pow(gameProb, 5) * std::pow(1 - gameProb, 5) * gameProb * gameProb;

		// Win in tiebreak (6-6, then win tiebreak)
		double tiebreakProb = tiebreakWinProbability(pointProb);
		result += binomialCoeff(12, 6) * std::pow(gameProb, 6) * std::pow(1 - gameProb, 6) * tiebreakProb;

		return result;
	}

	// Calculate tiebreak win probability (first to 7 with 2-point lead)
	static double tiebreakWinProbability(double pointProb) {
		if (pointProb == 0.5) return 0.5;

		double result = 0.0;
		// Win 7-0 to 7-5
		for (int i = 0; i <= 5; ++i) {
			result += binomialCoeff(6 + i, i) * std::pow(pointProb, 7) * std::pow(1 - pointProb, i);
		}

		// Extended tiebreak from 6-6 (simplified)
		double extendedProb = binomialCoeff(12, 6) * std::pow(pointProb, 6) * std::pow(1 - pointProb, 6);
		result += extendedProb * (std::pow(pointProb, 2) / (1 - 2 * pointProb * (1 - pointProb)));

		return result;
	}

	// Main prediction function
	static std::tuple<double, double, double> predictMatch(double elo1, double elo2, bool bestOf5 = false) {
		double pointProb = calculatePointWinProbability_2(elo1, elo2, bestOf5);
		//double pointProb = calculatePointWinProbability_2(elo1, elo2);
		double gameProb = gameWinProbability(pointProb);
		double setProb = setWinProbability(gameProb, pointProb);

		// Match win probability
		double matchProb = 0.0;

		auto matchWinProbabilityBestOf5 = [](double setProb) {
			double q = 1 - setProb;
			double result = 0.0;

			// Win 3-0: WWW
			result += std::pow(setProb, 3);

			// Win 3-1: 3 wins, 1 loss before win (choose 1 of 3 sets to be a loss)
			result += 3 * std::pow(setProb, 3) * q;

			// Win 3-2: 2 losses in first 4 sets, then a win
			result += 6 * std::pow(setProb, 3) * std::pow(q, 2);

			return result;
		};

		auto matchWinProbabilityBestOf3 = [](double setProb) {
			return std::pow(setProb, 2) + 2 * std::pow(setProb, 2) * (1 - setProb);
		};

		matchProb = bestOf5 ? matchWinProbabilityBestOf5(setProb) : matchWinProbabilityBestOf3(setProb);

		return std::make_tuple(gameProb, setProb, matchProb);
	}

	// Estimate expected number of games and sets
	static std::tuple<double, double> estimateMatchLength(double elo1, double elo2, bool bestOf5 = false) {
		auto [gameProb, setProb, matchProb] = predictMatch(elo1, elo2, bestOf5);

		// Expected number of sets in the match
		double expectedSets;
		if (bestOf5) {
			expectedSets = 3 * (std::pow(setProb, 3) + std::pow(1 - setProb, 3)) +
				4 * (3 * std::pow(setProb, 3) * (1 - setProb) + 3 * setProb * std::pow(1 - setProb, 3)) +
				5 * (6 * std::pow(setProb, 3) * std::pow(1 - setProb, 2) + 6 * std::pow(setProb, 2) * std::pow(1 - setProb, 3));
		}
		else {
			// For best of 3
			expectedSets = 2 * (std::pow(setProb, 2) + std::pow(1 - setProb, 2)) +
				3 * 2 * setProb * (1 - setProb);
		}

		double gamesPerSet = 6.0 / gameProb + 6.0 / (1 - gameProb);
		gamesPerSet = min(13.0, max(6.0, gamesPerSet));

		double expectedGames = expectedSets * gamesPerSet;

		return std::make_tuple(expectedGames, expectedSets);
	}

	// Simulate a match using Monte Carlo method
	static std::tuple<int, std::vector<int>, std::vector<std::pair<int, int>>> simulateMatch(
		double elo1, double elo2, bool bestOf5 = false, int simulations = 10000) {

		double pointProb = calculatePointWinProbability_2(elo1, elo2, bestOf5);
		//double pointProb = calculatePointWinProbability_1(elo1, elo2);
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<> dis(0.0, 1.0);

		int player1Wins = 0;
		int totalGames = 0;
		int totalSets = 0;
		std::vector<std::pair<int, int>> setScores;
		std::unordered_map<int, int> totalGamesCount;

		for (int sim = 0; sim < simulations; ++sim) {
			int sets1 = 0, sets2 = 0;
			int matchGames = 0;
			std::vector<std::pair<int, int>> matchSetScores;

			int setsToWin = bestOf5 ? 3 : 2;

			while (sets1 < setsToWin && sets2 < setsToWin) {
				auto [games1, games2] = simulateSet(pointProb, gen, dis);
				matchGames += games1 + games2;
				matchSetScores.push_back({ games1, games2 });

				if (games1 > games2) sets1++;
				else sets2++;
			}

			if (sets1 > sets2) player1Wins++;
			totalGamesCount[matchGames]++;
			totalSets += sets1 + sets2;

			if (sim == 0) setScores = matchSetScores; // Store first simulation as example
		}
		int mostOccuringGamesOutcome = 0;
		struct SimulatedGames {
			int games;
			int count;
		};
		std::vector<SimulatedGames> simulatedGames(totalGamesCount.size());
		int i = 0;
		for (const auto& [games, count] : totalGamesCount) {
			simulatedGames[i].games = games;
			simulatedGames[i++].count = count;
		}
		std::sort(std::begin(simulatedGames), std::end(simulatedGames), [](SimulatedGames& a, SimulatedGames& b) {return a.count > b.count; });
		std::vector<int> mostFreq(5);
		for (int i = 0; i < mostFreq.size(); ++i) {
			if (i < simulatedGames.size())
			 mostFreq[i] = simulatedGames[i].games;
		}

		return std::make_tuple(player1Wins, mostFreq, setScores);
	}


private:
	// Helper function for binomial coefficient
	static double binomialCoeff(int n, int k) {
		if (k > n - k) k = n - k;
		double result = 1;
		for (int i = 0; i < k; ++i) {
			result = result * (n - i) / (i + 1);
		}
		return result;
	}

	// Simulate a single set
	static std::pair<int, int> simulateSet(double pointProb, std::mt19937& gen,
		std::uniform_real_distribution<>& dis) {
		int games1 = 0, games2 = 0;

		while ((games1 < 6 && games2 < 6) || std::abs(games1 - games2) < 2) {
			if (games1 == 6 && games2 == 6) {
				// Tiebreak
				if (simulateTiebreak(pointProb, gen, dis)) games1++;
				else games2++;
				break;
			}

			// Simulate game
			if (simulateGame(pointProb, gen, dis)) games1++;
			else games2++;

			// Check for set win
			if ((games1 >= 6 || games2 >= 6) && std::abs(games1 - games2) >= 2) {
				break;
			}
		}

		return { games1, games2 };
	}

	// Simulate a single game
	static bool simulateGame(double pointProb, std::mt19937& gen,
		std::uniform_real_distribution<>& dis) {
		int points1 = 0, points2 = 0;

		while (true) {
			if (dis(gen) < pointProb) points1++;
			else points2++;

			if (points1 >= 4 && points1 - points2 >= 2) return true;
			if (points2 >= 4 && points2 - points1 >= 2) return false;
		}
	}

	// Simulate tiebreak
	static bool simulateTiebreak(double pointProb, std::mt19937& gen,
		std::uniform_real_distribution<>& dis) {
		int points1 = 0, points2 = 0;

		while (true) {
			if (dis(gen) < pointProb) points1++;
			else points2++;

			if (points1 >= 7 && points1 - points2 >= 2) return true;
			if (points2 >= 7 && points2 - points1 >= 2) return false;
		}
	}
};