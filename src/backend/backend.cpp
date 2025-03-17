#include "backend/backend.hpp"

//std
#include <fstream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/details.hpp"

namespace backend {
    void prepare_request_details(calc::request_t& what, const toml::value& source) {
        // agent

        what.agent.id = source.at("primary_agent_id").as_integer();
        std::string agent_as_str = std::to_string(what.agent.id);

        // wengine

        what.wengine.id = source.at("primary_wengine_id").as_integer();

        // rotation

        what.rotation.id = what.agent.id;

        // ddps and dds_count

        std::map<size_t, size_t> dds_count;

        for (const auto& [key, value] : source.at(agent_as_str).as_table()) {
            // ddps

            const auto& table = value.as_table();
            zzz::combat::DdpBuilder builder;

            uint64_t slot = std::stoul(key);
            uint64_t disc_id = table.at("set").as_integer();

            builder.set_slot(slot);
            builder.set_disc_id(disc_id);
            builder.set_rarity(zzz::convert::string_to_rarity(table.at("rarity").as_string()));

            builder.set_main_stat(
                zzz::convert::string_to_stat_type(table.at("main").as_string()),
                table.at("level").as_integer()
            );
            for (const auto& it : table.at("subs").as_array()) {
                const auto& array = it.as_array();
                builder.add_sub_stat(
                    zzz::convert::string_to_stat_type(array[0].as_string()),
                    array[1].as_integer()
                );
            }

            what.ddps[slot] = builder.get_product();

            // prepare dds_count

            if (auto jt = dds_count.find(disc_id); jt != dds_count.end())
                jt->second++;
            else
                dds_count[disc_id] = 1;
        }

        // dds

        for (const auto& [id, count] : dds_count) {
            if (count >= 2)
                what.dds.emplace(2, calc::request_t::cell_t<zzz::DdsDetails> {
                    .id = id,
                    .ptr = nullptr
                });
            if (count >= 4)
                what.dds.emplace(4, calc::request_t::cell_t<zzz::DdsDetails> {
                    .id = id,
                    .ptr = nullptr
                });
        }
    }
    void prepare_request_details(calc::request_t& what, const nlohmann::json& source) {
        // agent

        /*if (!source.contains("aid"))
            throw std::runtime_error("aid was not found");*/
        what.agent.id = source["aid"];

        // wengine

        what.wengine.id = source["wid"];

        // rotation

        what.rotation.id = source["rotation"];

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
            builder.set_rarity(zzz::convert::string_to_rarity(it["rarity"]));

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
            if (count >= 2)
                what.dds.emplace(2, calc::request_t::cell_t<zzz::DdsDetails> {
                    .id = id,
                    .ptr = nullptr
                });
            if (count >= 4)
                what.dds.emplace(4, calc::request_t::cell_t<zzz::DdsDetails> {
                    .id = id,
                    .ptr = nullptr
                });
        }
    }

    void prepare_request_composed(calc::request_t& what, ObjectManager& source) {
        auto agent_future = source.get("agents/" + std::to_string(what.agent.id));
        auto wengine_future = source.get("wengines/" + std::to_string(what.wengine.id));
        auto rotation_future = source.get("rotations/" + std::to_string(what.rotation.id));

        std::unordered_map<uint64_t, std::future<any_ptr>> dds_futures;
        for (const auto& cell : what.dds | std::views::values)
            dds_futures.try_emplace(cell.id, source.get("dds/" + std::to_string(cell.id)));

        try {
            what.agent.ptr = std::static_pointer_cast<zzz::AgentDetails>(agent_future.get());
            what.wengine.ptr = std::static_pointer_cast<zzz::WengineDetails>(wengine_future.get());
            what.rotation.ptr = std::static_pointer_cast<zzz::rotation_details>(rotation_future.get());

            std::unordered_map<uint64_t, zzz::DdsDetailsPtr> unique_dds;
            for (auto& it : what.dds | std::views::values) {
                zzz::DdsDetailsPtr ptr;

                if (auto jt = unique_dds.find(it.id); jt != unique_dds.end())
                    ptr = jt->second;
                else {
                    ptr = std::static_pointer_cast<zzz::DdsDetails>(dds_futures[it.id].get());
                    unique_dds.emplace(it.id, ptr);
                }

                it.ptr = ptr;
            }
        } catch (const std::runtime_error& e) {
            std::string message = lib::format("error: {}\n", e.what());
            std::cerr << message;
        }
    }
}

namespace backend {
    // getters

    ObjectManager& Backend::manager() {
        return m_manager;
    }

    // runners

    void Backend::run() {
        m_manager.launch();
        m_app.port(port)
            .run();
    }

    // requesters

    calc::request_t Backend::toml_to_request(const toml::value& toml) {
        calc::request_t result;

        prepare_request_details(result, toml);
        prepare_request_composed(result, m_manager);

        return result;
    }
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
                std::string name;

                switch (it.type()) {
                case toml::value_t::integer:
                    name = std::to_string(it.as_integer());
                    break;
                case toml::value_t::string:
                    name = it.as_string();
                    break;
                default:
                    throw std::runtime_error("wrong type");
                }

                allocated_objects++;
                m_manager.add_object(folder, folder + '/' + name);
                std::cout << lib::format("added object {}\n", folder + '/' + name);
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
            return "main page";
        });
        CROW_ROUTE(m_app, "/get_dmg").methods("POST"_method)([this](const crow::request& req) {
            crow::response response;

            try {
                auto json = nlohmann::json::parse(req.body);
                auto unpacked_request = json_to_request(json);
                auto calcs = request_calcs(unpacked_request);
                auto output = calcs_to_json(calcs).dump();

                response = { 200, std::move(output) };
            } catch (const std::exception& e) {
                response = { 400, e.what() };
            }

            return response;
        });
    }
}
