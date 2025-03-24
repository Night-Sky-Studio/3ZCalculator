#pragma once

//std
#include <map>

//utl

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
		StatsGrid() = default;

		StatsGrid(const StatsGrid& another);
		StatsGrid& operator=(const StatsGrid& another) noexcept;

		StatsGrid(StatsGrid&& another) noexcept;
		StatsGrid& operator=(StatsGrid&& another) noexcept;

		// json has to be an array
		// stats can be defined only once inside an array
		static StatsGrid make_from(const utl::Json& json, Tag mandatory_tag = Tag::Universal);

#ifdef DEBUG_STATUS
		tabulate::Table get_debug_table() const;
#endif

		// returns value by exact key
		double get(StatId id, Tag tag = Tag::Universal) const;
		// returns Universal + tag
		double get_summed(StatId id, Tag tag = Tag::Universal) const;

		double& at(StatId id, Tag tag = Tag::Universal);
		const double& at(StatId id, Tag tag = Tag::Universal) const;

		bool contains(StatId id, Tag tag = Tag::Universal) const;
		// TODO
		//bool contains_any(StatId id) const;

		// replaces ptr of stat if it exists
		void set(const StatPtr& stat);
		// adds value of stat if it exists
		// otherwise emplaces it
		void add(const StatPtr& stat);

		void add_regular(double value, StatId id, Tag tag);
		void add_relative(const std::string& formula, StatId id, Tag tag);

		void add(const StatsGrid& stat);

	protected:
		std::map<size_t, StatPtr> m_content;

	private:
		void _copy_from(const StatsGrid& another);

		StatPtr _sum_stats_as_copy(const StatPtr& l, const StatPtr& r);
	};
}
