#include "zzz/details/anomaly.hpp"

//std
#include <stdexcept>

namespace zzz::details {
	// Anomaly

	Anomaly Anomaly::make(std::string name, double scale, StatsGrid buffs) {
		return Anomaly(std::move(name), scale, std::move(buffs));
	}
	const Anomaly& Anomaly::get_standard_anomaly(Element element) {
		return _standard_anomalies.at(element);
	}

	const std::string& Anomaly::name() const { return m_name; }
	double Anomaly::scale() const { return m_scale; }
	const StatsGrid& Anomaly::buffs() const { return m_buffs; }
	bool Anomaly::can_crit() const { return m_can_crit; }

	Anomaly::Anomaly(std::string name, double scale, StatsGrid buffs) :
		m_name(std::move(name)),
		m_scale(scale),
		m_buffs(std::move(buffs)) {}

	const std::unordered_map<Element, Anomaly> Anomaly::_standard_anomalies = {
		{ Element::Physical, make("assault", 731.0) },
		{ Element::Fire, make("burn", 50.0 * 20) },
		{ Element::Ice, make("shatter", 500.0) },
		{ Element::Electric, make("shock", 125.0 * 10) },
		{ Element::Ether, make("corruption", 62.5 * 20) }
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
	AnomalyBuilder& AnomalyBuilder::add_buff(stat value) {
		m_product->m_buffs.emplace(value);
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
		return _is_set.name && _is_set.scale;
	}
	Anomaly&& AnomalyBuilder::get_product() {
		if (!is_built())
			throw std::runtime_error("you have to specify name and scale");

		return IBuilder::get_product();
	}
}
