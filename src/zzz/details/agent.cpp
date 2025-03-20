#include "zzz/details/agent.hpp"

//std
#include <stdexcept>

//library
#include "library/string_funcs.hpp"

namespace zzz::details {
    // Agent

    size_t Agent::is_skill_or_anomaly(const Agent& agent, const std::string& name) {
        size_t result;
        auto hashed_key = lib::hash(name);

        if (agent.m_skills.contains(hashed_key))
            result = 1;
        else if (agent.m_anomalies.contains(hashed_key))
            result = 2;
        else
            result = 0;

        return result;
    }

    uint64_t Agent::id() const { return m_id; }
    const std::string& Agent::name() const { return m_name; }
    Speciality Agent::speciality() const { return m_speciality; }
    Element Agent::element() const { return m_element; }
    Rarity Agent::rarity() const { return m_rarity; }
    const StatsGrid& Agent::stats() const { return m_stats; }
    const Skill& Agent::skill(const std::string& name) const { return m_skills.at(lib::hash(name)); }
    const Anomaly& Agent::anomaly(const std::string& name) const { return m_anomalies.at(lib::hash(name)); }

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
    AgentBuilder& AgentBuilder::add_stat(stat value) {
        m_product->m_stats.emplace(value);
        return *this;
    }
    AgentBuilder& AgentBuilder::set_stats(StatsGrid stats) {
        m_product->m_stats = std::move(stats);
        return *this;
    }
    AgentBuilder& AgentBuilder::add_skill(Skill skill) {
        auto hashed_key = lib::hash(skill.name());
        m_product->m_skills.emplace(hashed_key, std::move(skill));
        return *this;
    }
    AgentBuilder& AgentBuilder::add_anomaly(Anomaly anomaly) {
        auto hashed_key = lib::hash(anomaly.name());
        m_product->m_anomalies.emplace(hashed_key, std::move(anomaly));
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

    // AgentAdaptor

    Agent ToAgentConverter::from(const toml::value& data) const {
        AgentBuilder agent_builder;
        const auto& table = data.as_table();
        auto element = convert::string_to_element(table.at("element").as_string());

        agent_builder.set_id(table.at("id").as_integer());
        agent_builder.set_name(table.at("name").as_string());
        agent_builder.set_speciality(convert::string_to_speciality(table.at("speciality").as_string()));
        agent_builder.set_element(element);
        agent_builder.set_rarity((Rarity) table.at("rarity").as_integer());
        agent_builder.set_stats(global::to_stats_grid.from(table.at("stats")));

        for (const auto& [key, value] : table.at("skills").as_table()) {
            const auto& skill_table = value.as_table();
            SkillBuilder skill_builder;

            skill_builder.set_name(key);
            skill_builder.set_tag(convert::string_to_tag(skill_table.at("tag").as_string()));

            for (const auto& it : skill_table.at("scales").as_array()) {
                const toml::array& scale_data = it.as_array();
                skill_builder.add_scale(scale_data[0].as_floating(), scale_data[1].as_floating(),
                    scale_data.size() == 3 ? convert::string_to_element(scale_data[2].as_string()) : element);
            }

            if (auto it = skill_table.find("buffs"); it != skill_table.end())
                skill_builder.set_buffs(global::to_stats_grid.from(it->second));

            agent_builder.add_skill(skill_builder.get_product());
        }

        if (auto anomalies_it = table.find("anomalies"); anomalies_it != table.end()) {
            for (const auto& [key, value] : anomalies_it->second.as_table()) {
                const auto& anomaly_table = value.as_table();
                AnomalyBuilder anomaly_builder;

                anomaly_builder.set_name(key);
                anomaly_builder.set_scale(anomaly_table.at("scale").as_floating());

                auto can_crit_it = anomaly_table.find("can_crit");
                anomaly_builder.set_crit(can_crit_it != anomaly_table.end() ? can_crit_it->second.as_boolean() : false);

                if (auto it = anomaly_table.find("buffs"); it != anomaly_table.end())
                    anomaly_builder.set_buffs(global::to_stats_grid.from(it->second));

                agent_builder.add_anomaly(anomaly_builder.get_product());
            }
        } else
            agent_builder.add_anomaly(Anomaly::get_standard_anomaly(element));

        return agent_builder.get_product();
    }
}
