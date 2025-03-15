#pragma once

//std
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>

namespace backend {
    using any_ptr = std::shared_ptr<void>;

    using any_ptr_loader = std::function<any_ptr(const std::string&)>;
    //using any_ptr_deleter = std::function<bool(void*)>;

    class ObjectManager {
        struct object {
            any_ptr ptr;
            size_t utility_id;
            size_t cycles_since_last_usage;
        };

    public:
        struct utility_funcs {
            std::string folder;
            any_ptr_loader loader;
        };

        ObjectManager();

        template<typename T>
        const std::shared_ptr<T>& at(size_t key) const {
            std::shared_lock lock(*_mutex);
            auto result = std::static_pointer_cast<T>(_content.at(key).ptr);
            return std::move(result);
        }

        void add_utility_funcs(size_t utility_id, utility_funcs value);
        void add_utility_funcs(utility_funcs value);

        void add_object(size_t utility_id, size_t id);
        void add_object(const std::string& folder, size_t id);

    private:
        static constexpr size_t cycles_limit = 60;

        std::unordered_map<size_t, object> _content;
        std::unordered_map<size_t, utility_funcs> _utility_funcs;
        std::unique_ptr<std::shared_mutex> _mutex;
    };
}
