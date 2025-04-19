#pragma once

//lib
#include "library/cached_memory.hpp"

//crow
#include "crow/http_request.h"
#include "crow/http_response.h"

namespace backend::methods {
    std::string get_default();

    // TODO: make query options=[increment]
    crow::response post_refresh(lib::ObjectManager& manager);

    crow::response put_rotation(const crow::request& req);

    crow::response post_damage(const crow::request& req, lib::ObjectManager& manager);
}
