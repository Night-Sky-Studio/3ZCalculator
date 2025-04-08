#pragma once

//std
#include <any>
#include <atomic>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>

namespace lib {
    using MObjectPtr = std::shared_ptr<class MObject>;
    // helper for ObjectManager
    class MObject {
        friend class ObjectManager;

    public:
        explicit MObject(std::string fullname);
        virtual ~MObject() = default;

        template<typename T>
        T& as() { return *static_cast<T*>(_content.get()); }
        template<typename T>
        const T& as() const { return *static_cast<T*>(_content.get()); }

        template<typename T>
        void set(T value) {
            _content.reset(new T(std::move(value)));
        }

        bool is_allocated() const;

        // preferably make some private functions
        // which this overriden function will call in switch-case statement
        virtual bool load_from_string(const std::string& input, size_t mode) = 0;
        bool load_from_stream(std::istream& is, size_t mode);
        bool load(size_t mode);

    private:
        std::shared_ptr<void> _content = nullptr;
        // TODO: make atomic
        size_t _unused_period = 0;
        const std::string _fullname;
    };

    class ObjectManager {
    public:
        using ObjectMaker = std::function<MObjectPtr(std::string)>;

        static std::unordered_map<size_t, std::string> file_extensions;
        static void init_default_file_extensions() {
            file_extensions = {
                { 0, "" }, // none
                { 1, "json" },
                { 2, "toml" },
                { 3, "txt" }, // plane text
            };
        }

        // TODO: remake using lifetime in real time, not in cycles
        static constexpr size_t max_unused_period = 12ul;
        // TODO: make sleep time at most (while using multithreading)
        static constexpr auto sleep_time = std::chrono::seconds(10ul);

        explicit ObjectManager(size_t file_extension_id = 1);
        ~ObjectManager();

        MObjectPtr get(const std::string& key);
        std::future<MObjectPtr> get_async(std::string key);

        void add_object(const MObjectPtr& value);

        void free_memory();

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
