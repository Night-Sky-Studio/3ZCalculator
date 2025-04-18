#pragma once

//std
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>

//library
#include "library/builder.hpp"
#include "library/cached_memory.hpp"

//zzz
#include "zzz/details/anomaly.hpp"
#include "zzz/details/skill.hpp"
#include "zzz/enums.hpp"
#include "zzz/stats/basic.hpp"
#include "zzz/stats/grid.hpp"

namespace zzz::details {
    using Ability = std::variant<Skill, Anomaly>;

    class Agent {
        friend class AgentBuilder;

    public:
        uint64_t id() const;
        const std::string& name() const;
        Speciality speciality() const;
        Element element() const;
        Rarity rarity() const;
        const StatsGrid& stats() const;
        const StatsGrid& team_buffs() const;

        const Ability& ability(const std::string& name) const;

    protected:
        uint64_t m_id;
        std::string m_name;
        Speciality m_speciality;
        Element m_element;
        Rarity m_rarity;
        Faction m_faction;
        StatsGrid m_stats, m_team_buffs;
        std::unordered_map<size_t, Ability> m_abilities;
    };

    class AgentBuilder : public lib::IBuilder<Agent> {
    public:
        AgentBuilder& set_id(uint64_t id);
        AgentBuilder& set_name(std::string name);

        AgentBuilder& set_speciality(Speciality speciality);
        AgentBuilder& set_element(Element element);
        AgentBuilder& set_rarity(Rarity rarity);
        AgentBuilder& set_faction(Faction faction);

        AgentBuilder& add_stat(StatPtr&& value);
        AgentBuilder& set_stats(StatsGrid stats);

        AgentBuilder& add_team_buff(StatPtr&& value);
        AgentBuilder& set_team_buffs(StatsGrid stats);

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
            bool faction    : 1 = false;
        } _is_set;
    };
}

namespace zzz {
    using AgentDetails = details::Agent;
    using AbilityDetails = details::Ability;

    class Agent : public lib::MObject {
    public:
        explicit Agent(const std::string& name);

        AgentDetails& details();
        const AgentDetails& details() const;

        bool load_from_string(const std::string& input, size_t mode) override;
    };
    using AgentPtr = std::shared_ptr<Agent>;
}
