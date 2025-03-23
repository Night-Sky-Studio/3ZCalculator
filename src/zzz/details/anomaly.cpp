#include "zzz/details/anomaly.hpp"

//library
#include "library/format.hpp"
#include "library/string_funcs.hpp"

namespace zzz::details {
    // Anomaly

    std::pair<size_t, Anomaly> Anomaly::make_as_pair(
        std::string name,
        double scale,
        Element element,
        StatsGrid buffs) {
        size_t key = lib::hash(name);
        return { key, Anomaly { std::move(name), scale, element, std::move(buffs) } };
    }
    const Anomaly& Anomaly::get_standard_anomaly(std::string_view name) {
        size_t key = lib::hash(name);
        return standard_anomalies.at(key);
    }

    const std::string& Anomaly::name() const { return m_name; }
    double Anomaly::scale() const { return m_scale; }
    Element Anomaly::element() const { return m_element; }
    const StatsGrid& Anomaly::buffs() const { return m_buffs; }
    bool Anomaly::can_crit() const { return m_can_crit; }

    Anomaly::Anomaly(std::string name, double scale, Element element, StatsGrid buffs) :
        m_name(std::move(name)),
        m_scale(scale),
        m_element(element),
        m_buffs(std::move(buffs)) {
    }

    const std::unordered_map<size_t, Anomaly> Anomaly::standard_anomalies = {
        make_as_pair("assault", 731.0, Element::Physical),
        make_as_pair("burn", 50.0 * 20, Element::Fire),
        make_as_pair("shatter", 500.0, Element::Ice),
        make_as_pair("shock", 125.0 * 10, Element::Electric),
        make_as_pair("corruption", 62.5 * 20, Element::Ether)
    };

    // AnomalyBuilder

    AnomalyBuilder& AnomalyBuilder::set_name(std::string name) {
        m_product->m_name = std::move(name);
        _is_set.name = true;
        return *this;
    }

    AnomalyBuilder& AnomalyBuilder::set_scale(double scale) {
        m_product->m_scale = scale;
        _is_set.scale = true;
        return *this;
    }

    AnomalyBuilder& AnomalyBuilder::set_element(Element element) {
        m_product->m_element = element;
        _is_set.element = true;
        return *this;
    }
    AnomalyBuilder& AnomalyBuilder::set_element(std::string_view element) {
        return set_element(convert::string_to_element(element));
    }

    AnomalyBuilder& AnomalyBuilder::add_buff(const StatPtr& value) {
        m_product->m_buffs.add(value);
        return *this;
    }
    AnomalyBuilder& AnomalyBuilder::set_buffs(StatsGrid stats) {
        m_product->m_buffs = std::move(stats);
        return *this;
    }

    AnomalyBuilder& AnomalyBuilder::set_crit(bool can_crit) {
        m_product->m_can_crit = can_crit;
        return *this;
    }

    bool AnomalyBuilder::is_built() const {
        return _is_set.name
            && _is_set.scale
            && _is_set.element;
    }
    Anomaly&& AnomalyBuilder::get_product() {
        if (!is_built())
            throw RUNTIME_ERROR("you have to specify name, scale and element");

        return IBuilder::get_product();
    }
}
