#pragma once

//std
#include <memory>
#include <string>
#include <unordered_map>

//library
#include "library/builder.hpp"

//zzz
#include "zzz/enums.hpp"
#include "zzz/stats_grid.hpp"

namespace zzz::details {
	class Anomaly {
		friend class AnomalyBuilder;

	public:
		static std::pair<Element, Anomaly> make_as_pair(
			std::string name,
			double scale,
			Element element,
			StatsGrid buffs = {});
		static const Anomaly& get_standard_anomaly(Element element);

		Anomaly() = default;

		const std::string& name() const;
		double scale() const;
		Element element() const;
		const StatsGrid& buffs() const;
		bool can_crit() const;

	protected:
		std::string m_name;
		double m_scale;
		Element m_element;
		StatsGrid m_buffs;
		bool m_can_crit = false;

	private:
		static const std::unordered_map<Element, Anomaly> standard_anomalies;

		Anomaly(std::string name, double scale, Element element, StatsGrid buffs);
	};

	class AnomalyBuilder : public lib::IBuilder<Anomaly> {
	public:
		AnomalyBuilder& set_name(std::string name);

		AnomalyBuilder& set_scale(double scale);

		AnomalyBuilder& set_element(Element element);
		AnomalyBuilder& set_element(std::string_view element);

		AnomalyBuilder& add_buff(const StatPtr& value);
		AnomalyBuilder& set_buffs(StatsGrid stats);

		AnomalyBuilder& set_crit(bool can_crit);

		bool is_built() const override;
		Anomaly&& get_product() override;

	private:
		struct {
			bool name    : 1 = false;
			bool scale   : 1 = false;
			bool element : 1 = false;
		} _is_set;
	};
}

namespace zzz {
	using AnomalyDetails = details::Anomaly;
	using AnomalyDetailsPtr = std::shared_ptr<details::Anomaly>;
}
