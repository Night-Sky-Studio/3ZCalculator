#include "zzz/details/agent.hpp"

//std
#include <ranges>
#include <stdexcept>

//utl
#include "utl/json.hpp"

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

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
    const StatsGrid& Agent::team_buffs() const { return m_team_buffs; }

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
    AgentBuilder& AgentBuilder::set_element(Element element) {
        m_product->m_element = element;
        _is_set.element = true;
        return *this;
    }
    AgentBuilder& AgentBuilder::set_rarity(Rarity rarity) {
        m_product->m_rarity = rarity;
        _is_set.rarity = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::add_stat(StatPtr&& value) {
        m_product->m_stats.set(std::move(value));
        return *this;
    }
    AgentBuilder& AgentBuilder::set_stats(StatsGrid stats) {
        m_product->m_stats = std::move(stats);
        return *this;
    }

    AgentBuilder& AgentBuilder::add_team_buff(StatPtr&& value) {
        m_product->m_team_buffs.set(std::move(value));
        return *this;
    }
    AgentBuilder& AgentBuilder::set_team_buffs(StatsGrid stats) {
        m_product->m_team_buffs = std::move(stats);
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
            throw RUNTIME_ERROR("you have to specify id, name, speciality, element, rarity and stats");

        return IBuilder::get_product();
    }
}

#include <future>

namespace zzz {
    // Service

    AnomalyDetails make_anomaly_from(const std::string& key, const utl::Json& json, Element default_element) {
        const auto& table = json.as_object();
        details::AnomalyBuilder builder;

        builder.set_name(key);
        builder.set_scale(table.at("scale").as_floating());

        if (auto it = table.find("element"); it != table.end())
            builder.set_element((Element) it->second.as_string());
        else
            builder.set_element(default_element);

        if (auto it = table.find("buffs"); it != table.end()) {
            auto buffs = StatsGrid::make_from(it->second.as_array(), Tag::Anomaly);
            bool can_crit = buffs.contains({ StatId::CritRate, Tag::Anomaly })
                && buffs.contains({ StatId::CritDmg, Tag::Anomaly });

            builder.set_buffs(std::move(buffs));
            builder.set_crit(can_crit);
        }

        return builder.get_product();
    }

    SkillDetails::scale make_scale_from(const utl::Json& json, Element default_element) {
        const auto& array = json.as_array();
        SkillDetails::scale result;

        result.motion_value = array[0].as_floating();
        result.daze = array[1].as_floating();

        if (array.size() <= 2) {
            result.element = default_element;
            return result;
        }

        result.element = array[2].as_string();

        return result;
    }
    SkillDetails make_skill_from(const std::string& key, const utl::Json& json, Element default_element) {
        const auto& table = json.as_object();
        details::SkillBuilder builder;

        auto tag = (Tag) table.at("tag").as_string();

        builder.set_name(key);
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

    // json has to be root
    AgentDetails load_agent_from_json(const utl::Json& json) {
        const auto& table = json.as_object();
        details::AgentBuilder builder;

        // basic information

        auto element = (Element) table.at("element").as_string();

        builder.set_id(table.at("id").as_integral());
        builder.set_name(table.at("name").as_string());
        builder.set_speciality((Speciality) table.at("speciality").as_string());
        builder.set_element(element);
        builder.set_rarity(table.at("rarity").as_integral());

        // stats

        auto stats = StatsGrid::make_from(table.at("stats").as_array());
        builder.set_stats(std::move(stats));

        // team buffs

        if (auto it = table.find("team_buffs"); it != table.end()) {
            auto team_buffs = StatsGrid::make_from(it->second.as_array());
            builder.set_team_buffs(std::move(team_buffs));
        }

        // anomalies

        bool has_anomaly_redefinition = false;
        auto own_anomaly_name = AnomalyDetails::get_anomaly_by_element(element);

        if (auto it = table.find("anomalies"); it != table.end()) {
            for (const auto& [k, v] : it->second.as_object()) {
                auto anomaly = make_anomaly_from(k, v, element);

                if (own_anomaly_name == anomaly.name())
                    has_anomaly_redefinition = true;

                builder.add_anomaly(std::move(anomaly));
            }
        }
        if (!has_anomaly_redefinition)
            builder.add_anomaly(AnomalyDetails::get_standard_anomaly(own_anomaly_name));

        // skills

        for (const auto& [k, v] : table.at("skills").as_object())
            builder.add_skill(make_skill_from(k, v, element));

        return builder.get_product();
    }

    // Agent

    Agent::Agent(const std::string& name) :
        MObject(lib::format("agents/{}", name)) {
    }

    AgentDetails& Agent::details() { return as<AgentDetails>(); }
    const AgentDetails& Agent::details() const { return as<AgentDetails>(); }

    bool Agent::load_from_string(const std::string& input, size_t mode) {
        if (mode == 1) {
            auto json = utl::json::from_string(input);
            auto details = load_agent_from_json(json);
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
