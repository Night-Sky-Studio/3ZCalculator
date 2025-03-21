#pragma once

//std
#include <map>

//zzz
#include "stats.hpp"

#ifdef DEBUG_STATUS
#include "tabulate/table.hpp"
#endif

namespace zzz {
	class StatsGrid {
		using iterator = std::unordered_map<size_t, StatPtr>::iterator;
		using const_iterator = std::unordered_map<size_t, StatPtr>::const_iterator;
	public:
#ifdef DEBUG_STATUS
		tabulate::Table get_debug_table() const;
#endif

		// returns value by exact key
		double get(StatId id, Tag tag = Tag::Universal) const;
		// returns Universal + tag
		double get_summed(StatId id, Tag tag = Tag::Universal) const;

		double& at(StatId id, Tag tag = Tag::Universal);
		const double& at(StatId id, Tag tag = Tag::Universal) const;

		void add(const StatsGrid& another);

		void add(const RegularStat& stat);
		void add_regular(double value, StatId id, Tag tag);

		void add(const RelativeStat& stat);
		void add_relative(const std::string& formula, StatId id, Tag tag);

	protected:
		std::map<size_t, StatPtr> m_content;

	private:
		StatPtr _no_value;
	};
}
