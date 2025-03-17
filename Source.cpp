
#include "Basketball/bb_file_reader.h"



int main(int argc, char* argv[]) {
	//only basketball is currently supported.
	//Future sports include, Tennis, Baseball, Hockey and Football.-[;
	if (argc != 3) {
		printf("Usage: Rehoboam [Sport] [filename]\n");
		printf("[Sport] could be [t] for tennis, [b] for basketball\n");
		exit(1);
	}

	const char* file_path = argv[2];
	std::string sport = argv[1];
	std::cout << "sport = " << sport << "\n";
	std::cout << "length = " << sport.length() << "\n";
	if (sport == "b") {
		process_basketball_file(file_path);
	}
	else if (sport == "t") {
		//process_tennis_file(file_path)
	}
	else{
		printf("Sport unrecognized\n");
		exit(1);
	}
	return 0;
}