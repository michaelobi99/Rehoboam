
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <iomanip>
#include <optional>
#include "bb_predictors.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif


struct FootballTeam {
    std::string name;

    std::vector<int> goals_scored;
    std::vector<int> goals_conceded;
    std::string form;

    double avg_goals_scored{ 0.0 };
    double avg_goals_conceded{ 0.0 };

    std::optional<int> standing;
    std::optional<int> points;
    std::optional<int> match_played;
};


struct FootballH2H {
    int team1_score{ 0 };
    int team2_score{ 0 };

    // 1 = team1 was home
    // 2 = team1 was away
    int team1_location{ 0 };

    std::string date;
};


std::tuple<std::string, std::vector<int>>
split_football_history(const std::string& line) {

    size_t colon = line.find(':');

    if (colon == std::string::npos)
        return { "", {} };

    std::string left =
        trim(line.substr(0, colon));

    std::string numbers =
        trim(line.substr(colon + 1));

    std::vector<int> scores;

    std::stringstream ss(numbers);

    int value;

    while (ss >> value)
        scores.push_back(value);


    size_t pos = left.find(" goals scored");

    if (pos != std::string::npos) {
        left.erase(pos);
    }
    else {

        pos = left.find(" goals conceded");

        if (pos != std::string::npos)
            left.erase(pos);
    }

    return {
        trim(left),
        scores
    };
}


// ------------------------------------------------------------
// Parse:
// Home standing: 2 0
//
// Also handles:
// Home standing:
// ------------------------------------------------------------

void parse_standing(
    const std::string& line,
    const std::string& prefix,
    std::optional<int>& standing,
    std::optional<int>& points,
    std::optional<int>& match_played) {

    standing.reset();
    points.reset();

    size_t pos = line.find(prefix);

    if (pos == std::string::npos)
        return;

    std::string values =
        trim(line.substr(pos + prefix.size()));

    // Empty line after the colon
    if (values.empty())
        return;

    std::stringstream ss(values);

    int standing_value;
    int points_value;
    int match_played_value;

    if (ss >> standing_value)
        standing = standing_value;

    if (ss >> points_value)
        points = points_value;

    if (ss >> match_played_value)
        match_played = match_played_value;
}


void print_optional(const std::optional<int>& value,
    std::ostream& stream) {

    if (value.has_value())
        stream << *value;
    else
        stream << "--";
}


void process_football_file(const std::string& file_path) {

    std::fstream file{
        file_path,
        std::ios::in | std::ios::binary
    };

    if (!file.is_open()) {

        std::cerr
            << "Failed to open file\n";

        return;
    }


    std::string line;

    while (!file.eof()) {

        FootballTeam home;
        FootballTeam away;

        std::vector<FootballH2H> h2h;


        // ====================================================
        // HOME GOALS SCORED
        // ====================================================

        line.clear();

        while (
            std::getline(file, line)
            && line.size() <= 1
            )
            continue;

        if (file.eof())
            break;

        auto [home_name, home_scored] =
            split_football_history(line);

        home.name = home_name;
        home.goals_scored = home_scored;


        // ====================================================
        // HOME GOALS CONCEDED
        // ====================================================

        line.clear();

        while (
            std::getline(file, line)
            && line.size() <= 1
            )
            continue;

        auto [ignored_home_name, home_conceded] =
            split_football_history(line);

        home.goals_conceded = home_conceded;


        // ====================================================
        // AWAY GOALS SCORED
        // ====================================================

        line.clear();

        while (
            std::getline(file, line)
            && line.size() <= 1
            )
            continue;

        auto [away_name, away_scored] =
            split_football_history(line);

        away.name = away_name;
        away.goals_scored = away_scored;


        // ====================================================
        // AWAY GOALS CONCEDED
        // ====================================================

        line.clear();

        while (
            std::getline(file, line)
            && line.size() <= 1
            )
            continue;

        auto [ignored_away_name, away_conceded] =
            split_football_history(line);

        away.goals_conceded = away_conceded;


        // ====================================================
        // H2H COUNT
        // ====================================================

        line.clear();

        while (
            std::getline(file, line)
            && line.size() <= 1
            )
            continue;

        unsigned h2h_count = 0;

        {
            std::stringstream ss(line);

            std::string temp;

            ss >> temp >> h2h_count;
        }


        // ====================================================
        // H2H RESULTS
        //
        // Example:
        //
        // H2H 2
        // 0 2
        // 1 1
        // 1 1
        // 0 2
        //
        // First number = score
        // Second number = location
        //
        // 1 = Home
        // 2 = Away
        // ====================================================

        for (unsigned i = 0; i < h2h_count; ++i) {

            FootballH2H result;


            // Team 1

            line.clear();

            while (
                std::getline(file, line)
                && line.size() <= 1
                )
                continue;

            {
                std::stringstream ss(line);

                ss >> result.team1_score
                    >> result.team1_location
                    >> result.date;
            }


            // Team 2

            line.clear();

            while (
                std::getline(file, line)
                && line.size() <= 1
                )
                continue;

            {
                std::stringstream ss(line);

                int team2_location;

                ss >> result.team2_score
                    >> team2_location;
            }

            h2h.push_back(result);
        }


        // ====================================================
        // HOME STANDING
        // ====================================================

        line.clear();

        while (std::getline(file, line)&& line.size() <= 1)
            continue;

        parse_standing(
            line,"Home standing:", home.standing, home.points, home.match_played
        );


        // ====================================================
        // AWAY STANDING
        // ====================================================

        line.clear();

        while (std::getline(file, line) && line.size() <= 1)
            continue;

        parse_standing(
            line, "Away standing:", away.standing, away.points, away.match_played
        );


        // ====================================================
        // MATCH DETAILS
        // ====================================================

        std::string match_details;

        line.clear();

        while (
            std::getline(file, match_details)
            && match_details.size() <= 1
            )
            continue;


        // ====================================================
        // CALCULATE AVERAGES
        // ====================================================

        auto get_form = []
        (const std::vector<int>& goals_for, const std::vector<int>& goals_against)
            ->std::string {
            std::string form{ "" };
            if (std::size(goals_for) == std::size(goals_against)) {
                for (int i = 0; i < std::size(goals_for); ++i) {
                    if (goals_for[i] > goals_against[i]) form += 'W';
                    else if (goals_for[i] < goals_against[i]) form += 'L';
                    else form += 'D';
                }
            }
            return form;
        };

        home.avg_goals_scored =
            mean(home.goals_scored);

        home.avg_goals_conceded =
            mean(home.goals_conceded);

        home.form = get_form(home.goals_scored, home.goals_conceded);

        away.avg_goals_scored =
            mean(away.goals_scored);

        away.avg_goals_conceded =
            mean(away.goals_conceded);

        away.form = get_form(away.goals_scored, away.goals_conceded);


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



        // ====================================================
        // DISPLAY
        // ====================================================

        std::string design(75, '-');

        std::cout
            << design << "\n";

        std::cout
            << home.name
            << " vs "
            << away.name
            << " - "
            << match_details
            << "\n";

        std::cout
            << design << "\n";


        std::cout
            << std::fixed
            << std::setprecision(2);


        // Header

        std::cout
            << std::left
            << std::setw(25)
            << ""
            << std::setw(25)
            << home.name
            << std::setw(25)
            << away.name
            << "\n";


        // Average goals scored

        std::cout
            << std::setw(25)
            << "Avg Goals Scored"
            << std::setw(25)
            << home.avg_goals_scored
            << std::setw(25)
            << away.avg_goals_scored
            << "\n";


        // Average goals conceded

        std::cout
            << std::setw(25)
            << "Avg Goals Conceded"
            << std::setw(25)
            << home.avg_goals_conceded
            << std::setw(25)
            << away.avg_goals_conceded
            << "\n";

        double expected_home_goals =
            (home.avg_goals_scored + away.avg_goals_conceded) / 2.0;

        double expected_away_goals =
            (away.avg_goals_scored + home.avg_goals_conceded) / 2.0;

        double expected_total_goals =
            expected_home_goals + expected_away_goals;

        std::cout
            << std::setw(25)
            << "Expected Goals"
            << std::setw(25)
            << expected_home_goals
            << std::setw(25)
            << expected_away_goals
            << "\n";

        

        // Standing
        std::cout << std::setw(25) << "Standing";
        std::cout << std::setw(25);
        print_optional(home.standing, std::cout);
        print_optional(away.standing, std::cout);
        std::cout << "\n";

        // Matches played
        std::cout << std::setw(25) << "Matches Played";
        std::cout << std::setw(25);
        print_optional(home.match_played, std::cout);
        print_optional(away.match_played, std::cout);
        std::cout << "\n";

        // Points
        std::cout << std::setw(25) << "Points";
        std::cout << std::setw(25);
        print_optional(home.points, std::cout);
        print_optional(away.points, std::cout);
        std::cout << "\n";

        std::cout
            << std::setw(25)
            << "Expected Total Goals"
            << std::setw(25)
            << expected_total_goals
            << "\n";

        std::cout
            << std::setw(25)
            << "Form"
            << std::setw(25)
            << home.form
            << std::setw(25)
            << away.form
            << "\n";

        // ====================================================
        // RECENT H2H RESULTS
        // ====================================================

        if (!h2h.empty()) {

            std::cout
                << "\nRecent H2H results\n";

            int len_team1 = std::size(home.name);
            int len_team2 = std::size(away.name);
            std::cout
                << std::left
                << std::setw(len_team1 + 3)
                << home.name
                << std::setw(len_team2 + 3)
                << away.name
                << std::setw(10)
                << "Total"
                << std::setw(12)
                << "Home Team"
                << "Date"
                << "\n";

            std::cout
                << design
                << "\n";


            for (const auto& game : h2h) {

                std::string home_team = game.team1_location == 1 ? "Home" : "Away";

                std::cout
                    << std::left
                    << std::setw(len_team1 + 3)
                    << game.team1_score
                    << std::setw(len_team2 + 3)
                    << game.team2_score
                    << std::setw(10)
                    << game.team1_score
                    + game.team2_score
                    << std::setw(12)
                    << home_team
                    << game.date
                    << "\n";
            }
        }


        std::cout << design << "\n\n";
    }

    file.close();
}