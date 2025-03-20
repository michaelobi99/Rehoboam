#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <map>

#ifdef _MSC_VER
#include <Windows.h>
#endif


struct WTA_RANKING {
	std::string player;
	float ranking;
};

struct WOMEN_ELO {
	std::string player;
	float ranking;
	float hard_ranking;
	float clay_ranking;
	float grass_ranking;
};

struct ATP_RANKING {
	std::string player;
	float ranking;
};

struct MEN_ELO {
	std::string player;
	float ranking;
	float hard_ranking;
	float clay_ranking;
	float grass_ranking;
};


void process_tennis_file(std::string const& file_path) {

}