#pragma once

//frozen
#include "frozen/string.h"

//crow
#include "crow.h"

//backend
#include "backend/object_manager.hpp"

namespace backend {
	class Backend {
	public:
		static constexpr auto port = 8080;
		static constexpr frozen::string ip = "192.168.1.2";

		void init();
		void run();

	protected:
		crow::SimpleApp m_app;
	};
}
