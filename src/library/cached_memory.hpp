#pragma once

//std
#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>

//toml
#include "toml.hpp"

namespace lib {
    using any_ptr = std::shared_ptr<void>;
    using any_future = std::future<any_ptr>;

    using MObjectPtr = std::shared_ptr<class MObject>;
    // helper for ObjectManager
    class MObject {
        friend class ObjectManager;

    public:
        MObject(std::string fullname, size_t utility_id);
        virtual ~MObject() = default;

        template<typename T>
        std::shared_ptr<T> as() {
            auto result = std::static_pointer_cast<T>(m_ptr);
            return result;
        }
        any_ptr raw();

        bool is_allocated() const;

    protected:
        any_ptr m_ptr = nullptr;
        // TODO: make atomic
        size_t m_unused_period = 0;
        const std::string m_fullname;

        void set(any_ptr ptr);
        void set(nullptr_t);

        const any_ptr& operator*() const;
        any_ptr& operator*();

        const any_ptr& get() const;
        any_ptr& get();

        // preferably make some private functions
        // which this overriden function will call in switch-case statement
        virtual bool load_from_string(const std::string& input, size_t mode) = 0;
        bool load_from_stream(std::istream& is, size_t mode);
        bool load_from_file(size_t mode);
    };

    class ObjectManager {
    public:
        static std::unordered_map<size_t, std::string> file_extensions;
        // TODO: remake using lifetime in real time, not in cycles
        static constexpr size_t max_unused_period = 20ul;
        // TODO: make sleep time at most (while using multithreading)
        static constexpr auto sleep_time = std::chrono::seconds(1ul);

        explicit ObjectManager(size_t file_extension_id = 1);
        ~ObjectManager();

        MObjectPtr get(const std::string& key);
        std::future<MObjectPtr> get_async(std::string key);

        void add_object(const MObjectPtr& value);

        void launch();

        // deleted members

        ObjectManager(const ObjectManager&) = delete;
        ObjectManager(ObjectManager&&) = delete;
        ObjectManager& operator=(const ObjectManager&) = delete;
        ObjectManager& operator=(ObjectManager&&) = delete;

    protected:
        std::atomic_bool m_is_active = false, m_is_ended = false;
        size_t m_file_extension_id;
        std::unordered_map<size_t, MObjectPtr> m_content;

    private:
        MObjectPtr _get_logic(const std::string& key);

        void _launch_logic();
    };
}
