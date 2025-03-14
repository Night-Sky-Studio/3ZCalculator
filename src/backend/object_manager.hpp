#pragma once

//std
#include <functional>
#include <memory>
#include <mutex>

namespace backend {
	struct object_io {
		std::shared_ptr<void> content;
		std::mutex mutex;
	};

	class ObjectManager {
	private:

	};
}
