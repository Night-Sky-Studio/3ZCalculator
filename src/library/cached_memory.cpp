#include "library/cached_memory.hpp"

//std
#include <fstream>
#include <ranges>

//frozen
#include "frozen/string.h"

//lib
#include "library/format.hpp"
#include "library/string_funcs.hpp"

//crow
#include "crow/logging.h"

namespace global {
    extern std::string PATH;
}

namespace lib {
    // MObject

    MObject::MObject(std::string fullname) :
        _fullname(std::move(fullname)) {
    }

    bool MObject::is_allocated() const { return _content != nullptr; }

    bool MObject::load_from_stream(std::istream& is, size_t mode) {
        return load_from_string({ std::istreambuf_iterator(is), {} }, mode);
    }
    bool MObject::load(size_t mode) {
        if (_fullname.empty()) {
#ifdef DEBUG_STATUS
            CROW_LOG_INFO << "fullname isn't set";
#endif
        }

        auto path = lib::format("{}/data/{}.{}", global::PATH, _fullname, ObjectManager::file_extensions.at(mode));
        std::fstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
#ifdef DEBUG_STATUS
            CROW_LOG_INFO << lib::format("{} is not found", path);
#endif
            return false;
        }

        return load_from_stream(file, mode);
    }

    // ObjectManager

    std::unordered_map<size_t, std::string> ObjectManager::file_extensions = {};

    ObjectManager::ObjectManager(size_t file_extension_id) :
        m_file_extension_id(file_extension_id) {
        if (file_extensions.empty())
            init_default_file_extensions();
    }

    ObjectManager::~ObjectManager() {
        m_is_active = false;
        m_is_ended.wait(false);
    }

    MObjectPtr ObjectManager::get(const std::string& key) {
        return _get_logic(key);
    }
    std::future<MObjectPtr> ObjectManager::get_async(std::string key) {
        return std::async(std::launch::async, [this, key = std::move(key)] {
            return _get_logic(key);
        });
    }

    void ObjectManager::add_object(const MObjectPtr& value) {
        m_content.emplace(hash(value->_fullname), value);
    }

    void ObjectManager::launch() {
        m_is_active = true;
        std::thread thread(std::bind(&ObjectManager::_launch_logic, this));
        thread.detach();
    }

    MObjectPtr ObjectManager::_get_logic(const std::string& key) {
        auto it = m_content.find(hash(key));

        if (it == m_content.end())
            throw RUNTIME_ERROR(lib::format("{} doesn't exist", key));

        auto& object = it->second;
        object->_unused_period = 0;

        if (object->is_allocated())
            return object;

        object->load(m_file_extension_id);

#ifdef DEBUG_STATUS
        CROW_LOG_INFO << lib::format("{} is loaded", object->_fullname);
#endif

        return object;
    }

    void ObjectManager::_launch_logic() {
        while (m_is_active) {
            // resets object from memory when passed enough time
            for (auto& obj : m_content | std::views::values) {
                if (!obj->is_allocated())
                    continue;

                if (obj.use_count() == 1 && obj->_content.use_count() == 1)
                    obj->_unused_period++;

                if (obj->_unused_period == max_unused_period) {
                    obj->_content.reset();
#ifdef DEBUG_STATUS
                    CROW_LOG_INFO << lib::format("{} is deleted", obj->_fullname);
#endif
                }
            }

            std::this_thread::sleep_for(sleep_time);
        }

        m_is_ended = true;
        m_is_ended.notify_all();
    }
}
