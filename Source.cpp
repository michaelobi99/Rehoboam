
#include "Basketball/bb_fulltime_file_reader.h"
#include "Tennis/tennis_file_reader.h"
#include "Baseball/baseball_file_reader.h"

void usage() {
	printf("Usage: Rehoboam [Sport] [filename]\n");
	printf("[Sport] could be [t] for tennis, [bf] or [bq] for basketball\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	//only basketball is currently supported.
	//Future sports include, Tennis, Baseball, Hockey and Football.-[;
	if (argc != 3) {
		usage();
	}

	const char* file_path = argv[2];
	std::string sport = argv[1];
	if (sport == "basketball") {
		process_basketball_file(file_path, false);
	}
	else if (sport == "basketball_q") {
		process_basketball_file(file_path, true);
	}
	else if (sport == "tennis") {
		process_tennis_file(file_path);
	}
	else if (sport == "baseball") {
		process_baseball_file(file_path);
	}
	else{
		usage();
	}
	return 0;
}