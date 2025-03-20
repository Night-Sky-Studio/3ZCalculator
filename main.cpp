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

//zzz
#include "zzz/details/agent.hpp"

int main() {
	lib::ObjectManager::init_default_file_extensions();
	zzz::AgentPtr agent("1091");

	return 0;
}
