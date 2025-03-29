#include "backend/impl/details.hpp"

//std
#include <iostream>
#include <ranges>
#include <thread>

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

//zzz
#include "zzz/details.hpp"

//crow
#include "crow/logging.h"

namespace global {
    extern std::string PATH;
}

using namespace zzz;

namespace backend::details {
    // filesystem

    const std::unordered_map<std::string, object_maker> associated_folders = {
        {
            "agents",
            { .func = [](const std::string& name) { return std::make_shared<Agent>(name); } }
        },
        {
            "wengines",
            { .func = [](const std::string& name) { return std::make_shared<Wengine>(name); } }
        },
        {
            "dds",
            { .func = [](const std::string& name) { return std::make_shared<Dds>(name); } }
        },
        {
            "rotations",
            {
                .func = [](const std::string& name) { return std::make_shared<Rotation>(name); },
                .is_recursive = true
            }
        }
    };

    std::list<lib::MObjectPtr> regular_folder_iteration(const fs::directory_entry& entry, const maker_func& func) {
        std::list<lib::MObjectPtr> result;

        for (const auto& it : fs::directory_iterator(entry)) {
            // ignores other folders and non json files
            if (!it.is_regular_file() || it.path().extension() != ".json")
                continue;

            auto stem = it.path().stem().string();

            result.emplace_back(func(stem));
        }

        return result;
    }
    std::list<lib::MObjectPtr> recursive_folder_iteration(const fs::directory_entry& entry, const maker_func& func) {
        std::list<lib::MObjectPtr> result;

        auto source_dir = lib::format("{}/data/{}/", global::PATH, entry.path().filename().string());

        for (const auto& it : fs::recursive_directory_iterator(entry)) {
            if (!it.is_regular_file())
                continue;

            auto relative = it.path().lexically_relative(source_dir).relative_path().string();

            relative = relative.substr(0, relative.size() - relative.find_last_of('.') + 1);
            relative = lib::replace(relative, std::string(1, fs::path::preferred_separator), "/");

            result.emplace_back(func(relative));
        }

        return result;
    }

    // preparers

    void prepare_request_details(calc::request_t& what, const utl::Json& source) {
        const auto& table = source.as_object();

        // agent

        what.agent.id = table.at("aid").as_integral();

        // wengine

        what.wengine.id = table.at("wid").as_integral();

        // TODO: add rotation to the object manager and save it on disk

        if (!source["rotation"].is_array())
            what.rotation.id = table.at("rotation").as_integral();
        else {
            const auto& array = table.at("rotation").as_array();
            zzz::details::RotationBuilder builder;

            for (const auto& it : array) {
                auto splitted = lib::split_as_copy(it.as_string(), ' ');
                auto index = splitted.size() > 1 ? std::stoul(splitted[1]) : 0;
                builder.add_cell({ splitted[0], index });
            }

            what.rotation->set(builder.get_product());
        }

        // ddps and dds_count

        std::map<size_t, size_t> dds_count;
        size_t current_disk = 0;

        for (const auto& it : table.at("discs").as_array()) {
            const auto& v = it.as_object();
            combat::DdpBuilder builder;

            uint64_t disc_id = v.at("id").as_integral();

            const auto& stats = v.at("stats").as_array();
            const auto& levels = v.at("levels").as_array();

            builder.set_disc_id(disc_id);
            builder.set_slot(current_disk + 1);
            builder.set_rarity(v.at("rarity").as_integral());

            builder.set_main_stat(
                stats[0].is_integral() ? (StatId) stats[0].as_integral() : (StatId) stats[0].as_string(),
                levels[0].as_integral()
            );
            for (size_t i = 1; i < 5; i++)
                builder.add_sub_stat(
                    stats[i].is_integral() ? (StatId) stats[i].as_integral() : (StatId) stats[i].as_string(),
                    levels[i].as_integral()
                );

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

            auto& val = what.dds_list.emplace_back(id);
            what.dds_by_count.emplace(2, val.ptr);

            if (count < 4)
                continue;

            what.dds_by_count.emplace(4, val.ptr);
        }
    }
    void prepare_request_composed(calc::request_t& what, lib::ObjectManager& source) {
        auto agent_future = source.get_async(lib::format("agents/{}", what.agent.id));
        auto wengine_future = source.get_async(lib::format("wengines/{}", what.wengine.id));

        std::future<lib::MObjectPtr> rotation_future;
        if (what.rotation.ptr == nullptr)
            rotation_future = source.get_async(lib::format("rotations/{}/{}", what.agent.id, what.rotation.id));

        std::list<std::tuple<DdsPtr&, std::future<lib::MObjectPtr>>> dds_futures;
        for (auto& [id, ptr] : what.dds_list)
            dds_futures.emplace_back(ptr, source.get_async(lib::format("dds/{}", id)));

        try {
            what.agent.ptr = std::static_pointer_cast<Agent>(agent_future.get());
            what.wengine.ptr = std::static_pointer_cast<Wengine>(wengine_future.get());

            if (what.rotation.ptr == nullptr)
                what.rotation.ptr = std::static_pointer_cast<Rotation>(rotation_future.get());

            for (auto& [ptr, future] : dds_futures)
                ptr = std::static_pointer_cast<Dds>(future.get());
        } catch (const std::runtime_error& e) {
            CROW_LOG_ERROR << lib::format("error: {}", e.what());
        }
    }

    // requesters

    calc::request_t json_to_request(const utl::Json& json, lib::ObjectManager& manager) {
        calc::request_t result;

        prepare_request_details(result, json);
        prepare_request_composed(result, manager);

        return result;
    }

    utl::Json calcs_to_json(const calc::Calculator::result_t& calcs) {
        utl::Json result;
        const auto& [total, per_ability] = calcs; // [double, std::vector<double>]

        result["total"] = total;
        result["per_ability"] = per_ability;

        return result;
    }
    utl::Json calcs_to_detailed_json(const calc::Calculator::result_t& calcs) {
        utl::Json result;
        const auto& [total, per_ability] = calcs; // [double, std::vector<double>]

        result["total"] = total;

        return result;
    }

    utl::Json post_damage(const calc::request_t& request) {
        utl::Json json;

        auto [total_dmg, per_ability] = calc::Calculator::eval(request);
        json["total"] = total_dmg;
        json["per_ability"] = per_ability;

        return json;
    }
    utl::Json post_damage_detailed(const calc::request_t& request) {
        utl::Json json;

        auto [total_dmg, per_ability] = calc::Calculator::eval_detailed(request);
        json["total"] = total_dmg;
        utl::json::Array on_emplace;
        for (auto& [dmg, tag, name] : per_ability) {
            utl::json::Array line(3);
            line[0] = dmg;
            line[1] = (size_t) tag;
            line[2] = std::move(name);
            on_emplace.emplace_back(std::move(line));
        }
        json["per_ability"] = std::move(on_emplace);

        return json;
    }
}
