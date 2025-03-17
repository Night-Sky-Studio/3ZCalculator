#pragma once

//frozen
#include "frozen/string.h"

//toml11
#include "toml.hpp"

//nlohmann::json
#include "nlohmann/json.hpp"

//crow
#include "crow.h"

//calculator
#include "calc/calculator.hpp"
#include "calc/details.hpp"

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

        calc::request_t toml_to_request(const toml::value& toml);
        calc::request_t json_to_request(const nlohmann::json& json);

        static nlohmann::json calcs_to_json(const calc::Calculator::result_t& calcs);

        static calc::Calculator::result_t request_calcs(const calc::request_t& request);

    private:
        size_t _init_object_manager();
        void _init_crow_app();
    };
}
