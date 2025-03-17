#include "backend/backend.hpp"

//std
#include <fstream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>

//toml11
#include "toml.hpp"

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

        size_t current_disk = 0;
        std::map<size_t, size_t> dds_count;

        for (const auto& [key, value] : source.at(agent_as_str).as_table()) {
            // ddps

            const auto& table = value.as_table();
            zzz::combat::DdpBuilder builder;

            builder.set_slot(std::stoul(key));

            uint64_t disc_id = table.at("set").as_integer();
            builder.set_disc_id(disc_id);

            builder.set_rarity(zzz::convert::char_to_rarity(table.at("rarity").as_string()[0]));

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

            what.ddps[current_disk++] = builder.get_product();

            // prepare dds_count

            if (auto jt = dds_count.find(disc_id); jt != dds_count.end())
                jt->second++;
            else
                dds_count[disc_id] = 1;
        }

        // dds

        for (const auto& [count, id] : dds_count) {
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

        std::list<std::future<any_ptr>> dds_futures;
        for (const auto& cell : what.dds | std::views::values)
            dds_futures.emplace_back(source.get("dds/" + std::to_string(cell.id)));

        try {
            what.agent.ptr = std::static_pointer_cast<zzz::AgentDetails>(agent_future.get());
            what.wengine.ptr = std::static_pointer_cast<zzz::WengineDetails>(wengine_future.get());
            what.rotation.ptr = std::static_pointer_cast<zzz::rotation_details>(rotation_future.get());

            for (auto& future : dds_futures)
                auto ptr = std::static_pointer_cast<zzz::DdsDetails>(future.get());
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
            return "Hello, World!";
        });
        CROW_ROUTE(m_app, "/post_bin").methods("POST"_method)([](const crow::request& req) {
            const auto data = req.body;
            return crow::response("application/binary", data);
        });
    }
}
