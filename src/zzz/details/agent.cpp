#include "zzz/details/agent.hpp"

//std
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

    size_t Agent::is_skill_or_anomaly(const std::string& name) {
        size_t result;
        auto hashed_key = lib::hash(name);

        if (m_skills.contains(hashed_key))
            result = 1;
        else if (m_anomalies.contains(hashed_key))
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
    AgentBuilder& AgentBuilder::set_speciality(const std::string& speciality_str) {
        m_product->m_speciality = convert::string_to_speciality(speciality_str);
        _is_set.speciality = true;
        return *this;
    }

    AgentBuilder& AgentBuilder::set_element(Element element) {
        m_product->m_element = element;
        _is_set.element = true;
        return *this;
    }
    AgentBuilder& AgentBuilder::set_element(const std::string& element_str) {
        m_product->m_element = convert::string_to_element(element_str);
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
}

namespace zzz {
    AgentPtr::AgentPtr(const std::string& name) :
        MObject(lib::format("agents/{}", name)) {
        load_from_file(1);
    }

    AgentDetails load_from_json(const utl::json::Node& json) {
        details::AgentBuilder agent_builder;

        agent_builder.set_id(json["id"].get_integral());
        agent_builder.set_name(json["name"].get_string());
        agent_builder.set_speciality(json["speciality"].get_string());
        agent_builder.set_element(json["element"].get_string());
        agent_builder.set_rarity((Rarity) json["rarity"].get_integral());

        return agent_builder.get_product();
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
