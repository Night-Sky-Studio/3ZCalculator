#pragma once

//std
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <string>

//toml
#include "toml.hpp"

//library
#include "library/funcs.hpp"

namespace calculator {
    using any_ptr = std::shared_ptr<void>;

    using any_ptr_loader = std::function<any_ptr(const toml::value&)>;

    class ObjectManager {
        struct object {
            any_ptr ptr;
            std::string name;
            size_t utility_id;
            size_t cycles_since_last_usage;
        };

    public:
        struct utility_funcs {
            std::string folder;
            any_ptr_loader loader;
        };

        ~ObjectManager();

        std::future<any_ptr> get(const std::string& key);

        // blocks thread until object is gotten
        template<typename T>
        const std::shared_ptr<T>& at(const std::string& name) {
            auto hashed_key = lib::hash_string(name);
            auto future = get_as_future(hashed_key, std::launch::deferred);
            auto result = std::static_pointer_cast<T>(future.get());

            return std::move(result);
        }

        void add_utility_funcs(utility_funcs value);

        void add_object(const std::string& folder, const std::string& name);

        void launch();

    protected:
        std::atomic_bool m_is_active = false, m_is_ended = false;
        std::unordered_map<size_t, object> m_content;
        std::unordered_map<size_t, utility_funcs> m_utility_funcs;

        std::future<any_ptr> get_as_future(size_t key, std::launch policy);
    };
}
