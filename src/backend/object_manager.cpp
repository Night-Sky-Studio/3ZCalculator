#include "backend/object_manager.hpp"

//library
#include "library/funcs.hpp"

namespace backend {
    // ObjectManager

    ObjectManager::ObjectManager() :
        _mutex(new std::shared_mutex) {
    }

    void ObjectManager::add_utility_funcs(uint64_t utility_id, utility_funcs value) {
        std::unique_lock lock(*_mutex);
        _utility_funcs.emplace(utility_id, std::move(value));
    }
    void ObjectManager::add_utility_funcs(utility_funcs value) {
        auto hashed_key = lib::hash_string(value.folder);
        return add_utility_funcs(hashed_key, std::move(value));
    }

    void ObjectManager::add_object(size_t utility_id, size_t id) {
        std::unique_lock lock(*_mutex);
        const auto& util = _utility_funcs.at(utility_id);
        auto filename = util.folder + '/' + std::to_string(id) + ".toml";
        _content.emplace(id, object {
            .ptr = util.loader(filename),
            .utility_id = utility_id,
            .cycles_since_last_usage = 0
        });
    }
    void ObjectManager::add_object(const std::string& folder, size_t id) {
        return add_object(lib::hash_string(folder), id);
    }
}
