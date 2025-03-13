#pragma once

//std
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"
#include "zzz/details/anomaly.hpp"
#include "zzz/details/skill.hpp"

namespace zzz::details {
    // TODO: make skill and anomaly one unordered_map
    class Agent {
        friend class AgentBuilder;
        friend class AgentAdaptor;

    public:
        // 0 - none, 1 - skill, 2 - anomaly
        static size_t is_skill_or_anomaly(const Agent& agent, const std::string& name);

        uint64_t id() const;
        const std::string& name() const;
        Speciality speciality() const;
        Element element() const;
        Rarity rarity() const;
        const StatsGrid& stats() const;
        const Skill& skill(const std::string& name) const;
        const Anomaly& anomaly(const std::string& name) const;

    protected:
        uint64_t m_id;
        std::string m_name;
        Speciality m_speciality;
        Element m_element;
        Rarity m_rarity;
        StatsGrid m_stats;
        std::unordered_map<size_t, Skill> m_skills;
        std::unordered_map<size_t, Anomaly> m_anomalies;
    };

    class AgentBuilder : public lib::IBuilder<Agent> {
    public:
        AgentBuilder& set_id(uint64_t id);
        AgentBuilder& set_name(std::string name);
        AgentBuilder& set_speciality(Speciality speciality);
        AgentBuilder& set_element(Element element);
        AgentBuilder& set_rarity(Rarity rarity);
        AgentBuilder& add_stat(stat value);
        AgentBuilder& set_stats(StatsGrid stats);
        AgentBuilder& add_skill(Skill skill);
        AgentBuilder& add_anomaly(Anomaly anomaly);

        bool is_built() const override;
        Agent&& get_product() override;

    private:
        struct {
            bool id         : 1 = false;
            bool name       : 1 = false;
            bool speciality : 1 = false;
            bool element    : 1 = false;
            bool rarity     : 1 = false;
        } _is_set;
    };

    class AgentAdaptor : public lib::IConverter<Agent, toml::value> {
    public:
        Agent from(const toml::value& data) override;
    };
}

namespace zzz::global {
    static details::AgentAdaptor agent_adaptor;
}

namespace zzz {
    using AgentDetails = details::Agent;
    using AgentDetailsPtr = std::shared_ptr<details::Agent>;
}
