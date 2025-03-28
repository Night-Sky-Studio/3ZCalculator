//std
#include <string>

//library
#include "library/format.hpp"

//backend
#include "backend/backend.hpp"

namespace test {
    std::vector<size_t> dds_ids = {
        310, 311, 312, 313, 314, 315, 316, 318,
        319, 322, 323, 324, 325, 326, 327, 328
    };
    std::vector<size_t> agent_ids {
        1091, 1261
    };
    std::vector<std::string> rotation_ids {
        "1091/1"
    };
    std::vector<size_t> wengine_ids {
        13003, 13008, 13009, 14109, 14118
    };

    void logic(lib::ObjectManager& manager) {
        std::vector<lib::MObjectPtr> container;

        for (size_t id : dds_ids) {
            auto ptr = manager.get(lib::format("dds/{}", id * 100));
            container.emplace_back(ptr);
        }
        for (size_t id : agent_ids) {
            auto ptr = manager.get(lib::format("agents/{}", id));
            container.emplace_back(ptr);
        }
        for (const auto& id : rotation_ids) {
            auto ptr = manager.get(lib::format("rotations/{}", id));
            container.emplace_back(ptr);
        }
        for (size_t id : wengine_ids) {
            auto ptr = manager.get(lib::format("wengines/{}", id));
            container.emplace_back(ptr);
        }

        manager.free_memory();
    }
}

namespace global {
    std::string PATH = "./";
}

int main(int argc, char** argv) {
    if (argc > 1)
        global::PATH = argv[1];

    backend::Backend server;
    server.init();

    //test::logic(server.manager());

    server.run();

    return 0;
}
