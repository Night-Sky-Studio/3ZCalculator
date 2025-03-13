#include "zzz/details/drive_disc_set.hpp"

//std
#include <stdexcept>

namespace zzz::details {
    // DriveDiscSet

    uint64_t DriveDiscSet::id() const { return m_id; }
    const std::string& DriveDiscSet::name() const { return m_name; }

    const StatsGrid& DriveDiscSet::p2() const { return m_set_bonuses[0]; }
    const StatsGrid& DriveDiscSet::p4() const { return m_set_bonuses[1]; }

    // DriveDiscSetBuilder

    DdsBuilder& DdsBuilder::set_id(uint64_t id) {
        m_product->m_id = id;
        _is_set.id = true;
        return *this;
    }
    DdsBuilder& DdsBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }
    DdsBuilder& DdsBuilder::set_p2(StatsGrid bonus) {
        m_product->m_set_bonuses[0] = std::move(bonus);
        _is_set.p2 = true;
        return *this;
    }
    DdsBuilder& DdsBuilder::set_p4(StatsGrid bonus) {
        m_product->m_set_bonuses[1] = std::move(bonus);
        _is_set.p4 = true;
        return *this;
    }

    bool DdsBuilder::is_built() const {
        return _is_set.id
            && _is_set.name
            && _is_set.p2
            && _is_set.p4;
    }
    DriveDiscSet&& DdsBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, 2- and 4-piece bonuses");
        return IBuilder::get_product();
    }

    // DriveDiscSetAdaptor

    DriveDiscSet ToDdsConverter::from(const toml::value& data) {
        DdsBuilder builder;

        builder.set_id(data.at("id").as_integer());
        builder.set_name(data.at("name").as_string());

        const auto& set_bonuses = data.at("set_bonus").as_table();
        builder.set_p2(global::to_stats_grid.from(set_bonuses.at("p2")));
        builder.set_p4(global::to_stats_grid.from(set_bonuses.at("p4")));

        return builder.get_product();
    }
}
