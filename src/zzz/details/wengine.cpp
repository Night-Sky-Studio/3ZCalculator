#include "zzz/details/wengine.hpp"

//std
#include <stdexcept>

//frozen
#include "frozen/unordered_set.h"

//crow
#include "crow/logging.h"

//lib
#include "library/format.hpp"

namespace zzz::details::wengine_info {
    // ms - main stat
    constexpr frozen::unordered_set ms_limits = { StatId::AtkBase };
}

namespace zzz::details {
    // Wengine

    uint64_t Wengine::id() const { return m_id; }
    const std::string& Wengine::name() const { return m_name; }
    Speciality Wengine::speciality() const { return m_speciality; }
    const StatsGrid& Wengine::stats() const { return m_stats; }

    // WengineBuilder

    WengineBuilder& WengineBuilder::set_id(uint64_t id) {
        m_product->m_id = id;
        _is_set.id = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_rarity(Rarity rarity) {
        m_product->m_rarity = rarity;
        _is_set.rarity = true;
        return *this;
    }

    WengineBuilder& WengineBuilder::set_speciality(Speciality speciality) {
        m_product->m_speciality = speciality;
        _is_set.speciality = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_speciality(std::string_view speciality) {
        return set_speciality(convert::string_to_speciality(speciality));
    }

    // TODO: make proper check on main stat
    WengineBuilder& WengineBuilder::set_main_stat(const StatsGrid& main_stat) {
        /*if (!wengine_info::ms_limits.contains(main_stat))
            throw std::runtime_error("this main stat doesn't exist");*/

        m_product->m_stats.add(main_stat);
        _is_set.main_stat = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_sub_stat(const StatsGrid& sub_stat) {
        m_product->m_stats.add(sub_stat);
        _is_set.sub_stat = true;
        return *this;
    }
    WengineBuilder& WengineBuilder::set_passive_stats(const StatsGrid& stats) {
        m_product->m_stats.add(stats);
        _is_set.passive_stats = true;
        return *this;
    }

    bool WengineBuilder::is_built() const {
        return _is_set.id
            && _is_set.name
            && _is_set.rarity
            && _is_set.speciality
            && _is_set.main_stat
            && _is_set.sub_stat
            && _is_set.passive_stats;
    }
    Wengine&& WengineBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, speciality and stats [main and sub at least]");

        return IBuilder::get_product();
    }
}

namespace zzz {
    // Service

    // json has to be root
    WengineDetails load_from_json(const utl::Json& json) {
        const auto& table = json.as_object();
        details::WengineBuilder builder;

        builder.set_id(table.at("id").as_integral());
        builder.set_name(table.at("name").as_string());
        builder.set_rarity((Rarity) table.at("rarity").as_integral());
        builder.set_speciality(table.at("speciality").as_string());

        const auto& stats = table.at("stats").as_object();
        builder.set_main_stat(StatsGrid::make_from(stats.at("main")));
        builder.set_sub_stat(StatsGrid::make_from(stats.at("sub")));
        builder.set_passive_stats(StatsGrid::make_from(stats.at("passive")));

        return builder.get_product();
    }

    // Wengine

    Wengine::Wengine(const std::string& name) :
        MObject(lib::format("wengines/{}", name)) {
    }

    const WengineDetails& Wengine::details() const { return as<WengineDetails>(); }

    bool Wengine::load_from_string(const std::string& input, size_t mode) {
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
