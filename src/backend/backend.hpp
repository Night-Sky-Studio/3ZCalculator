#pragma once

//frozen
#include "frozen/string.h"

//toml11
#include "toml.hpp"

//crow
#include "crow.h"

//calculator
#include "calculator/calculator.hpp"
#include "calculator/details.hpp"

//backend
#include "backend/object_manager.hpp"

namespace backend {
    class Backend {
    public:
        static constexpr auto port = 8080;
        static constexpr frozen::string ip = "192.168.1.2";

        ObjectManager& manager();

        void init();
        void run();

    protected:
        ObjectManager m_manager;
        crow::SimpleApp m_app;

        calculator::eval_data_details request_eval_data_details(const toml::value& toml);
        calculator::eval_data_composed request_eval_data_composed(const calculator::eval_data_details& details);
        calculator::Calculator::result_t request_calcs();

    private:
        size_t _init_object_manager();
        void _init_crow_app();
    };
}
