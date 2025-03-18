//backend
#include "backend/backend.hpp"

int main() {
    backend::Backend server("server.log");

    server.init();
    server.run();

    return 0;
}
