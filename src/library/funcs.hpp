#pragma once

//std
#include <string>

namespace lib::ext {
#include "library/crc64.hpp"
}

namespace lib {
	inline size_t hash_string(const std::string& what) {
		return ext::crc64(0, what.data(), what.size());
	}
	constexpr size_t hash_cstr(const char* what, size_t length) {
		return ext::crc64(0, what, length);
	}
}
