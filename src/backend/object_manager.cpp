#include "backend/object_manager.hpp"

//std
#include <chrono>
#include <fstream>
#include <ranges>
#include <stdexcept>

#ifdef DEBUG_STATUS
#include <iostream>
#endif

namespace backend {
    static constexpr auto cycles_limit = 2ul;
    static constexpr auto sleep_time = std::chrono::seconds(1);

    // ObjectManager

    ObjectManager::~ObjectManager() {
        m_is_active = false;
        m_is_ended.wait(false);
    }

    std::future<any_ptr> ObjectManager::get(const std::string& key) {
        return get_as_future(lib::hash_string(key), std::launch::async);
    }

    void ObjectManager::add_utility_funcs(utility_funcs value) {
        auto hashed_key = lib::hash_string(value.folder);
        m_utility_funcs.emplace(hashed_key, std::move(value));;
    }

    void ObjectManager::add_object(const std::string& folder, const std::string& name) {
        m_content.emplace(lib::hash_string(name), object {
            .ptr = nullptr,
            .name = name,
            .utility_id = lib::hash_string(folder),
            .cycles_since_last_usage = 0
        });
    }

    void ObjectManager::launch() {
        m_is_active = true;
        std::thread thread([this] {
            while (m_is_active) {
                // resets object from memory when passed enough time
                for (auto& [key, obj] : m_content) {
                    if (!obj.ptr)
                        continue;

                    if (obj.ptr.use_count() == 1)
                        obj.cycles_since_last_usage++;
                    else
                        obj.cycles_since_last_usage = 0;

                    if (obj.cycles_since_last_usage == cycles_limit) {
                        obj.ptr = nullptr;
#ifdef DEBUG_STATUS
                        std::string message = lib::format("{} object is deleted", key);
                        std::cerr << message;
#endif
                    }
                }

                std::this_thread::sleep_for(sleep_time);
            }

            m_is_ended = true;
            m_is_ended.notify_all();
        });
        thread.detach();
    }

    std::future<any_ptr> ObjectManager::get_as_future(size_t key, std::launch policy) {
        return std::async(policy, [this, key] {
            auto it = m_content.find(key);
            auto& object = it->second;

            if (it == m_content.end())
                throw std::runtime_error("object doesn't exist");

            if (it->second.ptr.get() != nullptr)
                return object.ptr;

            const auto& util = m_utility_funcs.at(object.utility_id);

            std::string path = object.name + ".toml";
            std::fstream file(path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                std::string message = lib::format("file {} is not found", path);
#ifdef DEBUG_STATUS
                std::cerr << message;
#endif
                throw std::runtime_error(message);
            }
            auto toml = toml::parse(file);
            file.close();

            object.ptr = util.loader(toml);
#ifdef DEBUG_STATUS
            std::string message = lib::format("object {}/{} is loaded\n", util.folder, key);
            std::cerr << message;
#endif
            object.cycles_since_last_usage = 0;

            return object.ptr;
        });
    }
}
