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

    DriveDiscSetBuilder& DriveDiscSetBuilder::set_id(uint64_t id) {
        m_product->m_id = id;
        _is_set.id = true;
        return *this;
    }
    DriveDiscSetBuilder& DriveDiscSetBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }
    DriveDiscSetBuilder& DriveDiscSetBuilder::set_p2(StatsGrid bonus) {
        m_product->m_set_bonuses[0] = std::move(bonus);
        _is_set.p2 = true;
        return *this;
    }
    DriveDiscSetBuilder& DriveDiscSetBuilder::set_p4(StatsGrid bonus) {
        m_product->m_set_bonuses[1] = std::move(bonus);
        _is_set.p4 = true;
        return *this;
    }

    bool DriveDiscSetBuilder::is_built() const {
        return _is_set.id
            && _is_set.name
            && _is_set.p2
            && _is_set.p4;
    }
    DriveDiscSet&& DriveDiscSetBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, 2- and 4-piece bonuses");
        return IBuilder::get_product();
    }

    // DriveDiscSetAdaptor

    DriveDiscSet DriveDiscSetAdaptor::from(const toml::value& data) {
        DriveDiscSetBuilder builder;

        builder.set_id(data.at("id").as_integer());
        builder.set_name(data.at("name").as_string());

        const auto& set_bonuses = data.at("set_bonus").as_table();
        builder.set_p2(global::stats_grid_adaptor.from(set_bonuses.at("p2")));
        builder.set_p4(global::stats_grid_adaptor.from(set_bonuses.at("p4")));

        return builder.get_product();
    }
}
