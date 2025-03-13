#pragma once

namespace lib {
	template<typename T1, typename T2> class IAdaptor {
	public:
		virtual ~IAdaptor() = default;

		virtual T1 to_t1(const T2& data) = 0;
		virtual T2 to_t2(const T1& data) = 0;
	};
}
