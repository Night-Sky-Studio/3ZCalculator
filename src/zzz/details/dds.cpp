#include "zzz/details/dds.hpp"

//std
#include <stdexcept>

//crow
#include "crow/logging.h"

//lib
#include "library/format.hpp"

namespace zzz::details {
    // Dds

    uint64_t Dds::id() const { return m_id; }
    const std::string& Dds::name() const { return m_name; }

    const StatsGrid& Dds::p2() const { return m_set_bonuses[0]; }
    const StatsGrid& Dds::p4() const { return m_set_bonuses[1]; }

    // DdsBuilder

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
    Dds&& DdsBuilder::get_product() {
        if (!is_built())
            throw std::runtime_error("you have to specify id, name, 2- and 4-piece bonuses");
        return IBuilder::get_product();
    }
}

namespace zzz {
    // Service

    DdsDetails load_from_json(const utl::Json& json) {
        const auto& table = json.as_object();
        details::DdsBuilder builder;

        builder.set_id(table.at("id").as_integral());
        builder.set_name(table.at("name").as_string());

        const auto& set_bonuses = table.at("set_bonus").as_array();
        builder.set_p2(StatsGrid::make_from(set_bonuses[0]));
        builder.set_p4(StatsGrid::make_from(set_bonuses[1]));

        return builder.get_product();
    }

    // Dds

    Dds::Dds(const std::string& fullname) :
        MObject(lib::format("dds/{}")) {
    }

    const DdsDetails& Dds::details() const { return as<DdsDetails>(); }

    bool Dds::load_from_string(const std::string& input, size_t mode) {
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
