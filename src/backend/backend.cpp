#include "backend/backend.hpp"

//std
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

//toml11
#include "toml.hpp"

//library
#include "library/funcs.hpp"

//zzz
#include "zzz/details.hpp"

//calculator
#include "calculator/converters.hpp"

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

    calculator::eval_data_details Backend::request_eval_data_details(const toml::value& toml) {
        return global::to_eval_data_details.from(toml);
    }
    calculator::eval_data_composed Backend::request_eval_data_composed(const calculator::eval_data_details& details) {
        calculator::eval_data_composed result;

        auto agent_future = m_manager.get(details.agent_id);
        auto wengine_future = m_manager.get(details.wengine_id);
        auto rotation_future = m_manager.get(details.rotation_id);

        std::map<size_t, size_t> dds_count;
        for (const auto& it : details.drive_discs) {
            if (auto jt = dds_count.find(it.disc_id()); jt != dds_count.end())
                jt->second++;
            else
                dds_count[it.disc_id()] = 1;
        }

        std::list<std::future<any_ptr>> dds_futures;
        for (const auto& [id, count] : dds_count) {
            if (count >= 2) {
                auto key = "dds/" + std::to_string(id);
                dds_futures.emplace_back(m_manager.get(key));
            }
        }

        result.agent = std::static_pointer_cast<zzz::AgentDetails>(agent_future.get());
        result.wengine = std::static_pointer_cast<zzz::WengineDetails>(wengine_future.get());
        result.rotation = std::static_pointer_cast<zzz::rotation_details>(rotation_future.get());

        for (auto& future : dds_futures) {
            auto ptr = std::static_pointer_cast<zzz::DdsDetails>(future.get());
            auto id = dds_count.at(ptr->id());

            if (id >= 2)
                result.dds.emplace(2, ptr);
            if (id >= 4)
                result.dds.emplace(4, ptr);
        }

        return result;
    }
    calculator::Calculator::result_t Backend::request_calcs() {}

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
