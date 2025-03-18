#include "backend/object_manager.hpp"

//std
#include <chrono>
#include <fstream>
#include <ranges>
#include <stdexcept>

//library
#include "library/funcs.hpp"

#ifdef DEBUG_STATUS
#include "crow/logging.h"
#endif

namespace backend {
    static constexpr auto cycles_limit = 15ul;
    static constexpr auto sleep_time = std::chrono::seconds(1);

    // ObjectManager

    ObjectManager::~ObjectManager() {
        m_is_active = false;
        m_is_ended.wait(false);
    }

    std::future<any_ptr> ObjectManager::get(std::string key) {
        return std::async(std::launch::async, [this, key = std::move(key)] {
            return this->_get_or_load(key);
        });
    }

    void ObjectManager::add_utility_funcs(utility_funcs value) {
        auto hashed_key = lib::hash_string(value.folder);
        m_utility_funcs.emplace(hashed_key, std::move(value));
    }

    void ObjectManager::add_object(const std::string& folder, const std::string& name) {
        m_content.emplace(lib::hash_string(name), object {
            .ptr = nullptr,
            .cycles_since_last_usage = 0,
            .name = name,
            .utility_id = lib::hash_string(folder)
        });
    }

    void ObjectManager::launch() {
        m_is_active = true;
        std::thread thread(std::bind(&ObjectManager::_launch_code, this));
        thread.detach();
    }

    any_ptr ObjectManager::_get_or_load(const std::string& key) {
        auto it = m_content.find(lib::hash_string(key));

        if (it == m_content.end())
            throw std::runtime_error(lib::format("object {} doesn't exist", key));

        auto& object = it->second;
        if (object.ptr != nullptr)
            return object.ptr;

        const auto& [folder, loader] = m_utility_funcs.at(object.utility_id);

        std::string path = object.name + ".toml";
        std::fstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error(lib::format("file {} is not found", path));
        auto toml = toml::parse(file);
        file.close();

        object.ptr = loader(toml);

#ifdef DEBUG_STATUS
        CROW_LOG_INFO << lib::format("{} is loaded", object.name);
#endif

        return object.ptr;
    }

    void ObjectManager::_launch_code() {
        while (m_is_active) {
            // resets object from memory when passed enough time
            for (auto& obj : m_content | std::views::values) {
                if (!obj.ptr)
                    continue;

                if (obj.ptr.use_count() == 1)
                    obj.cycles_since_last_usage++;
                else
                    obj.cycles_since_last_usage = 0;

                if (obj.cycles_since_last_usage == cycles_limit) {
                    obj.ptr = nullptr;
#ifdef DEBUG_STATUS
                    CROW_LOG_INFO << lib::format("{} object is deleted", obj.name);
#endif
                }
            }

            std::this_thread::sleep_for(sleep_time);
        }

        m_is_ended = true;
        m_is_ended.notify_all();
    }
}
