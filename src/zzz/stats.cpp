#include "zzz/stats.hpp"

//lib
#include "library/format.hpp"

namespace zzz {
    // IStat

    IStat::IStat(size_t type) :
        _type(type) {
    }

    double IStat::base() const {
        return m_base;
    }
    StatId IStat::id() const {
        return m_id;
    }
    Tag IStat::tag() const {
        return m_tag;
    }

    // RegularStat

    StatPtr RegularStat::make(double base, StatId id, Tag tag) {
        RegularStat result;

        result.m_base = base;
        result.m_id = id;
        result.m_tag = tag;

        return std::make_shared<IStat>(std::move(result));
    }

    StatPtr RegularStat::make_from_floating(const utl::Json& json) {
        RegularStat result;

        result.m_base = json.as_floating();
        result.m_id = convert::string_to_stat_id(json.key());

        return std::make_shared<IStat>(std::move(result));
    }
    StatPtr RegularStat::make_from_object(const utl::Json& json) {
        RegularStat result;

        result.m_base = json["val"].as_floating();
        result.m_id = convert::string_to_stat_id(json.key());
        result.m_tag = convert::string_to_tag(json.value_or<std::string>("tag", "universal"));

        return std::make_shared<IStat>(std::move(result));
    }

    RegularStat::RegularStat() :
        IStat(1) {
    }

    StatPtr RegularStat::copy_as_ptr() const {
        RegularStat result;

        result.m_base = m_base;
        result.m_id = m_id;
        result.m_tag = m_tag;

        return std::make_shared<IStat>(std::move(result));
    }

    double RegularStat::value() const {
        return m_base;
    }

    // RelativeStat

    // TODO
    StatPtr RelativeStat::make_from_string(const utl::Json& json) {
        return nullptr;
    }
    // TODO
    StatPtr RelativeStat::make_from_object(const utl::Json& json) {
        return nullptr;
    }

    RelativeStat::RelativeStat() :
        IStat(2) {
    }

    // StatFactory

    void StatFactory::init_default() {
        default_type_name = "regular";
        m_makers = {
            { "regular 1", RegularStat::make_from_object },
            { "regular 5", RegularStat::make_from_floating },
            { "relative 1", RegularStat::make_from_object },
            { "relative 3", RelativeStat::make_from_string }
        };
    }

    bool StatFactory::add_maker(std::string key, StatMaker value) {
        auto [_, flag] = m_makers.emplace(std::move(key), std::move(value));
        return flag;
    }

    StatPtr StatFactory::make(const utl::Json& json) {
        const auto& key = json.is_object()
            ? json.value_or<std::string>("type", default_type_name) + ' ' + std::to_string((size_t) json.type())
            : default_type_name;
        auto it = m_makers.find(key);
        return it != m_makers.end() ? it->second(json) : nullptr;
    }
}
