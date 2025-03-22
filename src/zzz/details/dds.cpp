#include "zzz/details/dds.hpp"

//std
#include <stdexcept>

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
