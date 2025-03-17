#include "backend/backend.hpp"

namespace backend {
    void Backend::init() {
        CROW_ROUTE(m_app, "/")([] {
            return "Hello, World!";
        });
        CROW_ROUTE(m_app, "/post_bin").methods("POST"_method)([](const crow::request& req) {
            const auto data = req.body;
            return crow::response("application/binary", data);
        });
    }
    void Backend::run() {
        m_app.port(port)
            .run();
    }
}
