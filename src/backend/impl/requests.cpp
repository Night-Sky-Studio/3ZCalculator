#include "backend/impl/requests.hpp"

//std
#include <filesystem>
#include <string>

//utl
#include "utl/json.hpp"

//lib
#include "library/format.hpp"

//backend
#include "backend/impl/details.hpp"

namespace fs = std::filesystem;

namespace backend::methods {
    std::string get_default() {
        return "3Z Calculator Backend";
    }

    crow::response post_refresh(lib::ObjectManager& manager) {
        crow::response response;

        manager.clear();
        details::prepare_object_manager(manager);

        return response;
    }

    crow::response put_rotation(const crow::request& req) {
        crow::response response;

        try {
            fs::path ar_folder = lib::format("data/rotations/{}",
                req.url_params.get("aid"));

            if (!fs::exists(ar_folder))
                fs::create_directory(ar_folder);

            auto filename = lib::format("{}/{}.json",
                ar_folder.string(), req.url_params.get("id"));

            std::fstream file(filename, std::ios::out | std::ios::trunc);
            if (!file.is_open())
                throw FMT_RUNTIME_ERROR("file {} is not found", filename);

            auto json = utl::json::from_string(req.body);
            file << json.to_string(utl::json::Format::PRETTY);

            response.code = 200;
        } catch (const std::exception& e) {
            response = { 500, e.what() };
        }

        return response;
    }

    crow::response post_damage(const crow::request& req, lib::ObjectManager& manager) {
        crow::response response;

        try {
            std::string type = req.url_params.get("type");
            calc::request_t unpacked_request;
            utl::Json for_assign;

            details::prepare_request_details(unpacked_request, utl::json::from_string(req.body));
            details::prepare_request_composed(unpacked_request, manager);

            if (type.empty()) {
                auto [total_dmg, per_ability] = calc::Calculator::eval(unpacked_request);

                for_assign["total"] = total_dmg;
                for_assign["per_ability"] = per_ability;
            } else if (type == "detailed") {
                auto [total_dmg, per_ability] = calc::Calculator::eval_detailed(unpacked_request);

                utl::json::Array temp;
                for (auto& [dmg, tags, name] : per_ability) {
                    utl::json::Array line(3);
                    line[0] = dmg;

                    if (tags.size() == 1)
                        line[1] = (size_t) tags.front();
                    else {
                        for (size_t i = 0; i < tags.size(); i++)
                            line[1][i] = (size_t) tags[i];
                    }

                    line[2] = std::move(name);
                    temp.emplace_back(std::move(line));
                }
                for_assign["per_ability"] = std::move(temp);
            } else
                throw FMT_RUNTIME_ERROR("invalid request \"/damage?type={}\"", type);

            response.body = for_assign.to_string(utl::json::Format::MINIMIZED);

            response.code = 200;
        } catch (const std::exception& e) {
            response = { 500, e.what() };
        }

        return response;
    }
}
