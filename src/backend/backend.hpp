#pragma once

//std
#include <fstream>

//crow
#include "crow/app.h"

//library
#include "library/cached_memory.hpp"

//backend
#include "library/logger.hpp"

namespace backend {
    class Backend {
    public:
        static constexpr auto max_thread_load = 2ul;
        static constexpr auto port = 5101;

        Backend() = default;
        ~Backend();

        lib::ObjectManager& manager();

        void init();
        void run();

    protected:
        lib::ObjectManager m_manager;
        crow::SimpleApp m_app;
        Logger m_logger;
        std::optional<std::fstream> m_log_file;

    private:
        void _init_logger(bool use_file);
        size_t _init_object_manager();
        void _init_crow_app();
    };
}
