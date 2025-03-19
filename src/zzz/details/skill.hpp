#pragma once

//std
#include <memory>
#include <span>
#include <string>
#include <vector>

//library
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats.hpp"

namespace zzz::details {
    // level for abilities is always 12
    class Skill {
        friend class SkillBuilder;

    public:
        struct scale {
            double motion_value;
            double daze;
            Element element;
        };

        const std::string& name() const;
        Tag tag() const;
        std::span<const scale> scales() const;
        const StatsGrid& buffs() const;

    protected:
        std::string m_name;
        Tag m_tag;
        std::vector<scale> m_scales;
        StatsGrid m_buffs;
    };

    class SkillBuilder : public lib::IBuilder<Skill> {
    public:
        SkillBuilder& set_name(std::string name);
        SkillBuilder& set_tag(Tag tag);
        SkillBuilder& add_scale(Skill::scale value);
        SkillBuilder& add_scale(double motion_value, double daze, Element element);
        SkillBuilder& add_buff(stat buff);
        SkillBuilder& set_buffs(StatsGrid buffs);

        bool is_built() const override;
        Skill&& get_product() override;

    private:
        struct {
            bool name : 1 = false;
            bool tag  : 1 = false;
        } _is_set;
    };
}

namespace zzz {
    using SkillDetails = details::Skill;
    using SkillDetailsPtr = std::shared_ptr<details::Skill>;
}
