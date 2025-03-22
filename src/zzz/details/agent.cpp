#include "zzz/details/agent.hpp"

//std
#include <stdexcept>

//utl
#include "utl/json.hpp"

//library
#include <ranges>

#include "library/format.hpp"
#include "library/string_funcs.hpp"
#include "zzz/stats_grid.hpp"

#ifdef DEBUG_STATUS
#include "crow/logging.h"
#endif

namespace zzz::details {
    // Agent

    uint64_t Agent::id() const { return m_id; }
    const std::string& Agent::name() const { return m_name; }
    Speciality Agent::speciality() const { return m_speciality; }
    Element Agent::element() const { return m_element; }
    Rarity Agent::rarity() const { return m_rarity; }
    const StatsGrid& Agent::stats() const { return m_stats; }

    const Ability& Agent::ability(const std::string& name) const { return m_abilities.at(lib::hash(name)); }

    // AgentBuilder

    AgentBuilder& AgentBuilder::set_id(uint64_t id) {
        m_product->m_id = id;
        _is_set.id = true;
        return *this;
    }
    AgentBuilder& AgentBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::set_speciality(Speciality speciality) {
        m_product->m_speciality = speciality;
        _is_set.speciality = true;
        return *this;
    }
    AgentBuilder& AgentBuilder::set_speciality(std::string_view speciality_str) {
        m_product->m_speciality = convert::string_to_speciality(speciality_str);
        _is_set.speciality = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::set_element(Element element) {
        m_product->m_element = element;
        _is_set.element = true;
        return *this;
    }
    AgentBuilder& AgentBuilder::set_element(std::string_view element_str) {
        m_product->m_element = convert::string_to_element(element_str);
        _is_set.element = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::set_rarity(Rarity rarity) {
        m_product->m_rarity = rarity;
        _is_set.rarity = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::add_stat(const StatPtr& value) {
        m_product->m_stats.set(value);
        return *this;
    }
    AgentBuilder& AgentBuilder::set_stats(StatsGrid stats) {
        m_product->m_stats = std::move(stats);
        return *this;
    }

    AgentBuilder& AgentBuilder::add_skill(Skill skill) {
        auto hashed_key = lib::hash(skill.name());
        m_product->m_abilities.emplace(hashed_key, std::move(skill));
        return *this;
    }
    AgentBuilder& AgentBuilder::add_anomaly(Anomaly anomaly) {
        auto hashed_key = lib::hash(anomaly.name());
        m_product->m_abilities.emplace(hashed_key, std::move(anomaly));
        return *this;
    }

    bool AgentBuilder::is_built() const {
        return _is_set.id
            && _is_set.name
            && _is_set.speciality
            && _is_set.element
            && _is_set.rarity;
    }
    Agent&& AgentBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, speciality, element, rarity and stats");

        return IBuilder::get_product();
    }
}

namespace zzz {
    AgentPtr::AgentPtr(const std::string& name) :
        MObject(lib::format("agents/{}", name)) {
        load_from_file(1);
    }

    AnomalyDetails make_anomaly_from(const utl::json::Node& json, Element default_element) {
        const auto& table = json.as_object();
        details::AnomalyBuilder builder;

        builder.set_name(json.key_as_copy());
        builder.set_scale(table.at("scale").as_floating());

        if (auto it = table.find("element"); it != table.end())
            builder.set_element(it->second.as_string());
        else
            builder.set_element(default_element);

        if (auto it = table.find("buffs"); it != table.end()) {
            auto buffs = StatsGrid::make_from(it->second, Tag::Anomaly);
            bool can_crit = buffs.contains(StatId::CritRate, Tag::Anomaly)
                && buffs.contains(StatId::CritDmg, Tag::Anomaly);

            builder.set_buffs(std::move(buffs));
            builder.set_crit(can_crit);
        }

        return builder.get_product();
    }

    SkillDetails::scale make_scale_from(const utl::json::Node& json, Element default_element) {
        const auto& array = json.as_array();
        SkillDetails::scale result;

        result.motion_value = array[0].as_floating();
        result.daze = array[1].as_floating();

        if (array.size() <= 2) {
            result.element = default_element;
            return result;
        }

        result.element = convert::string_to_element(array[2].as_string());

        return result;
    }
    SkillDetails make_skill_from(const utl::json::Node& json, Element default_element) {
        const auto& table = json.as_object();
        details::SkillBuilder builder;

        auto tag = convert::string_to_tag(table.at("tag").as_string());

        builder.set_name(json.key_as_copy());
        builder.set_tag(tag);

        if (auto it = table.find("scale"); it != table.end()) {
            builder.add_scale(make_scale_from(it->second, default_element));
        } else if (it = table.find("scales"); it != table.end()) {
            for (const auto& jt : it->second.as_array())
                builder.add_scale(make_scale_from(jt, default_element));
        }

        if (auto it = table.find("buffs"); it != table.end()) {
            auto buffs = StatsGrid::make_from(it->second, tag);
            builder.set_buffs(std::move(buffs));
        }

        return builder.get_product();
    }

    AgentDetails load_from_json(const utl::json::Node& json) {
        details::AgentBuilder builder;

        auto element = convert::string_to_element(json["element"].as_string());

        builder.set_id(json["id"].as_integral());
        builder.set_name(json["name"].as_string());
        builder.set_speciality(json["speciality"].as_string());
        builder.set_element(element);
        builder.set_rarity((Rarity) json["rarity"].as_integral());

        auto stats = StatsGrid::make_from(json["stats"]);
        builder.set_stats(std::move(stats));

        for (const auto& value : json["anomalies"].as_object() | std::views::values)
            builder.add_anomaly(make_anomaly_from(value, element));

        for (const auto& value : json["skills"].as_object() | std::views::values)
            builder.add_skill(make_skill_from(value, element));

        return builder.get_product();
    }

    bool AgentPtr::load_from_string(const std::string& input, size_t mode) {
        if (mode == 1) {
            auto json = utl::json::from_string(input);
            auto details = load_from_json(json);
            set(std::move(details));
        } else {
#ifdef DEBUG_STATUS
            CROW_LOG_ERROR << lib::format("extension_id {} isn't defined", mode);
#endif
            return false;
        }

        return true;
    }
}
