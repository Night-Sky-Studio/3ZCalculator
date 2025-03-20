#include "backend/backend.hpp"

//std
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

//zzz
#include "zzz/details.hpp"

#ifdef DEBUG_STATUS
//crow
#include "crow/logging.h"
#endif

namespace backend {
    void prepare_request_details(calc::request_t& what, const nlohmann::json& source) {
        // agent

        what.agent.id = source["aid"];

        // wengine

        what.wengine.id = source["wid"];

        // rotation

        if (!source["rotation"].is_array())
            what.rotation.id = source["rotation"];
        else {
            what.rotation.ptr = std::make_shared<zzz::rotation_details>();

            const auto& rotations = source["rotation"].get<std::vector<std::string>>();
            for (const auto& it : rotations) {
                auto splitted = lib::split_as_copy(it, ' ');
                auto index = splitted.size() > 1
                    ? std::stoul(splitted[1])
                    : 0;
                what.rotation.ptr->emplace_back(splitted[0], index);
            }

            // TODO: add rotation to the object manager and save it on disk
        }

        // ddps and dds_count

        std::map<size_t, size_t> dds_count;
        size_t current_disk = 0;

        for (const auto& it : source["discs"]) {
            zzz::combat::DdpBuilder builder;

            uint64_t disc_id = it["id"];
            const std::vector<uint32_t>& stats = it["stats"];
            const std::vector<uint32_t>& levels = it["levels"];

            builder.set_disc_id(disc_id);
            builder.set_slot(current_disk + 1);
            builder.set_rarity(it["rarity"]);

            builder.set_main_stat((zzz::StatType) stats[0], levels[0]);
            for (size_t i = 1; i < 5; i++)
                builder.add_sub_stat((zzz::StatType) stats[i], levels[i]);

            what.ddps[current_disk++] = builder.get_product();

            // prepare dds_count

            if (auto jt = dds_count.find(disc_id); jt != dds_count.end())
                jt->second++;
            else
                dds_count[disc_id] = 1;
        }

        // dds

        for (const auto& [id, count] : dds_count) {
            if (count < 2)
                continue;

            auto& [_, ptr] = what.dds.uniques.emplace_back(id, nullptr);
            what.dds.by_count.emplace(2, &ptr);

            if (count < 4)
                continue;

            what.dds.by_count.emplace(4, &ptr);
        }
    }

    // remake with unordered_map or list
    void prepare_request_composed(calc::request_t& what, lib::ObjectManager& source) {
        auto agent_future = source.get(lib::format("agents/{}", what.agent.id));
        auto wengine_future = source.get(lib::format("wengines/{}", what.wengine.id));

        std::future<any_ptr> rotation_future;
        if (what.rotation.ptr == nullptr)
            rotation_future = source.get(lib::format("rotations/{}/{}", what.agent.id, what.rotation.id));

        std::list<std::tuple<zzz::DdsDetailsPtr*, std::future<any_ptr>>> dds_futures;
        for (auto& [id, ptr] : what.dds.uniques)
            dds_futures.emplace_back(&ptr, source.get(lib::format("dds/{}", id)));

        try {
            what.agent.ptr = std::static_pointer_cast<zzz::AgentDetails>(agent_future.get());
            what.wengine.ptr = std::static_pointer_cast<zzz::WengineDetails>(wengine_future.get());

            if (what.rotation.ptr == nullptr)
                what.rotation.ptr = std::static_pointer_cast<zzz::rotation_details>(rotation_future.get());

            for (auto& [ptr, future] : dds_futures)
                *ptr = std::static_pointer_cast<zzz::DdsDetails>(future.get());
        } catch (const std::runtime_error& e) {
            CROW_LOG_ERROR << lib::format("error: {}", e.what());
        }
    }
}

namespace backend {
    // getters

    Backend::Backend(const std::string& logger_file) :
        m_logger(logger_file) {
    }

    ObjectManager& Backend::manager() {
        return m_manager;
    }

    // runners

    void Backend::run() {
        m_manager.launch();

        uint16_t max_threads = std::thread::hardware_concurrency();
        m_app.port(port)
            .concurrency(max_threads < max_thread_load ? max_threads : max_thread_load)
            .run();
    }

    // requesters

    calc::request_t Backend::json_to_request(const nlohmann::json& json) {
        calc::request_t result;

        prepare_request_details(result, json);
        prepare_request_composed(result, m_manager);

        return result;
    }

    nlohmann::json Backend::calcs_to_json(const calc::Calculator::result_t& calcs) {
        nlohmann::json result;
        const auto& [total, per_ability] = calcs; // [double, std::vector<double>]

        result["total"] = total;
        result["per_ability"] = per_ability;

        return result;
    }

    calc::Calculator::result_t Backend::request_calcs(const calc::request_t& request) {
        return calc::Calculator::eval(request);
    }

    // initializers

    void Backend::init() {
        crow::logger::setHandler(&m_logger);
        crow::logger::setLogLevel(crow::LogLevel::INFO);

        _init_object_manager();
        _init_crow_app();
    }

    size_t Backend::_init_object_manager() {
        size_t allocated_objects = 0;
        std::fstream file("loadable_objects.toml", std::ios::in | std::ios::binary);
        auto toml = toml::parse(file);
        file.close();

        auto add_objects_and_loader = [&](const std::string& folder, const any_ptr_loader& loader) {
            m_manager.add_utility_funcs({
                .folder = folder,
                .loader = loader
            });
            for (const auto& it : toml.at(folder).as_array()) {
                std::string name = folder + '/';

                switch (it.type()) {
                case toml::value_t::integer:
                    name += std::to_string(it.as_integer());
                    break;
                case toml::value_t::string:
                    name += it.as_string();
                    break;
                default:
                    throw std::runtime_error("wrong type");
                }

                allocated_objects++;
                m_manager.add_object(folder, name);
#ifdef DEBUG_STATUS
                CROW_LOG_INFO << lib::format("added object {}", name);
#endif
            }
        };

        add_objects_and_loader("agents", [](const toml::value& toml) {
            auto result = std::make_shared<zzz::AgentDetails>(global::to_agent.from(toml));
            return std::static_pointer_cast<void>(result);
        });
        add_objects_and_loader("wengines", [](const toml::value& toml) {
            auto result = std::make_shared<zzz::WengineDetails>(global::to_wengine.from(toml));
            return std::static_pointer_cast<void>(result);
        });
        add_objects_and_loader("dds", [](const toml::value& toml) {
            auto result = std::make_shared<zzz::DdsDetails>(global::to_dds.from(toml));
            return std::static_pointer_cast<void>(result);
        });
        add_objects_and_loader("rotations", [](const toml::value& toml) {
            auto result = std::make_shared<zzz::rotation_details>(global::to_rotation.from(toml));
            return std::static_pointer_cast<void>(result);
        });

        return allocated_objects;
    }
    void Backend::_init_crow_app() {
        CROW_ROUTE(m_app, "/")([] {
            return "3Z Calculator Backend";
        });
        CROW_ROUTE(m_app, "/stop").methods("GET"_method)([this](const crow::request& req) {
            m_app.stop();
            return crow::response(503, "server.stop() was called");
        });
        CROW_ROUTE(m_app, "/get_dmg").methods("POST"_method)([this](const crow::request& req) {
            crow::response response;

            try {
                auto json = nlohmann::json::parse(req.body);
                auto unpacked_request = json_to_request(json);
                auto calcs = request_calcs(unpacked_request);
                auto output = calcs_to_json(calcs).dump();

                response = { 200, std::move(output) };
                response.add_header("Access-Control-Allow-Origin", "*");
            } catch (const std::exception& e) {
                response = { 400, e.what() };
                response.add_header("Access-Control-Allow-Origin", "*");
            }

            return response;
        });
    }
}
