#include "zzz/details/skill.hpp"

//lib
#include "library/format.hpp"

namespace zzz::details {
    // Skill

    const std::string& Skill::name() const { return m_name; }
    Tag Skill::tag() const { return m_tag; }
    std::span<const Skill::scale> Skill::scales() const { return { m_scales.data(), m_scales.size() }; }
    const StatsGrid& Skill::buffs() const { return m_buffs; }

    // SkillBuilder

    SkillBuilder& SkillBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }

    SkillBuilder& SkillBuilder::set_tag(Tag tag) {
        m_product->m_tag = tag;
        _is_set.tag = true;
        return *this;
    }
    SkillBuilder& SkillBuilder::set_tag(std::string_view tag) {
        return set_tag(convert::string_to_tag(tag));
    }

    SkillBuilder& SkillBuilder::add_scale(Skill::scale value) {
        m_product->m_scales.emplace_back(value);
        return *this;
    }
    SkillBuilder& SkillBuilder::add_scale(double motion_value, double daze, Element element) {
        return add_scale({
            .motion_value = motion_value,
            .daze = daze,
            .element = element
        });
    }
    SkillBuilder& SkillBuilder::add_scale(double motion_value, double daze, std::string_view element) {
        return add_scale({
            .motion_value = motion_value,
            .daze = daze,
            .element = convert::string_to_element(element)
        });
    }

    SkillBuilder& SkillBuilder::add_buff(const StatPtr& buff) {
        m_product->m_buffs.add(buff);
        return *this;
    }
    SkillBuilder& SkillBuilder::set_buffs(StatsGrid buffs) {
        m_product->m_buffs = std::move(buffs);
        return *this;
    }

    bool SkillBuilder::is_built() const {
        return _is_set.name
            && _is_set.tag;
    }
    Skill&& SkillBuilder::get_product() {
        if (!is_built())
            throw RUNTIME_ERROR("you have to specify name and tag");

        return IBuilder::get_product();
    }
}
