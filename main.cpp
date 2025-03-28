//std
#include <string>

//library
#include "library/format.hpp"

//backend
#include "backend/backend.hpp"

namespace global {
    std::string PATH = "./";
}

int main(int argc, char** argv) {
    if (argc > 1)
        global::PATH = argv[1];

    backend::Backend server;
    server.init();
    server.run();

    return 0;
}
