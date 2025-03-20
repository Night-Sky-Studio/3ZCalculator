#pragma once

//std
#include <cstdint>
#include <string>
#include <unordered_map>

//library
#include "library/builder.hpp"
#include "library/cached_memory.hpp"

//zzz
#include "utl/json.hpp"
#include "zzz/details/anomaly.hpp"
#include "zzz/details/skill.hpp"
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    // TODO: make skill and anomaly one unordered_map
    class Agent {
        friend class AgentBuilder;

    public:
        // 0 - none, 1 - skill, 2 - anomaly
        size_t is_skill_or_anomaly(const std::string& name);

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
        AgentBuilder& set_speciality(const std::string& speciality_str);

        AgentBuilder& set_element(Element element);
        AgentBuilder& set_element(const std::string& element_str);

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
}

namespace zzz {
    using AgentDetails = details::Agent;

    class AgentPtr : public lib::MObject {
    public:
        explicit AgentPtr(const std::string& name);

    protected:
        bool load_from_string(const std::string& input, size_t mode) override;
    };
}
