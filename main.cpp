////backend
//#include "backend/backend.hpp"
//
//int main() {
//    backend::Backend server("server.log");
//
//    server.init();
//    server.run();
//
//    return 0;
//}

//std
#include <string>

//zzz
#include "zzz/details/agent.hpp"

namespace global {
	std::string PATH = "data";
}

int main(int argc, char** argv) {
	if (argc > 1)
		global::PATH = argv[1];

	lib::ObjectManager::init_default_file_extensions();
	zzz::StatFactory::init_default();

	return 0;
}
