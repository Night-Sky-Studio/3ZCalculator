#pragma once

//frozen
#include "frozen/string.h"

//utl
#include "utl/json.hpp"

//crow
#include "crow/app.h"

//library
#include "library/cached_memory.hpp"

//calculator
#include "calc/calculator.hpp"
#include "calc/details.hpp"

//backend
#include "library/logger.hpp"

namespace backend {
    class Backend {
    public:
        static constexpr auto max_thread_load = 2ul;
        static constexpr auto port = 5101;
        static constexpr frozen::string ip = "192.168.1.2";

        explicit Backend(const std::string& logger_file);
        lib::ObjectManager& manager();

        void init();
        void run();

    protected:
        lib::ObjectManager m_manager;
        crow::SimpleApp m_app;
        Logger m_logger;

    private:
        size_t _init_object_manager();
        void _init_crow_app();
    };
}
