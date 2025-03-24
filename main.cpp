//std
#include <string>

//math
#include "math/parser.hpp"

//backend
#include "backend/backend.hpp"

namespace global {
	std::string PATH = "./";
}

int main(int argc, char** argv) {
	if (argc > 1)
		global::PATH = argv[1];

	/*backend::Backend server;

	server.init();
	server.run();*/

	std::string s = "4 + 18 / (9 - 3)";
	auto tokens = math::Parser::tokenize(s);
	auto rpn = math::Parser::shunting_yard_algorithm(tokens);

	return 0;
}
