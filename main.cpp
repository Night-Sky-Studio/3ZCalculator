//backend
#include "backend/backend.hpp"

int main() {
    backend::Backend server;

    server.init();
    server.run();

    return 0;
}
